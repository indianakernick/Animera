//
//  color picker widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color picker widget.hpp"

#include <cmath>
#include "config.hpp"
#include <QtGui/qevent.h>
#include <QtCore/qtimer.h>
#include "global font.hpp"
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include <QtGui/qvalidator.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qgridlayout.h>

namespace {

struct HSV {
  int h, s, v;
};

QRgb hsv2rgb(const qreal h, const qreal s, const qreal v) {
  QColor color;
  color.setHsvF(h / 360.0, s / 100.0, v / 100.0);
  return color.rgba();
}

void renderBorder(QPainter &painter, const QRect innerRect, const QRect outerRect) {
  painter.setClipRegion(QRegion{outerRect}.subtracted(innerRect));
  painter.drawRect(outerRect);
}

QPixmap initColorBitmap(
  const QString &pathP,
  const QString &pathS,
  const QColor colorP,
  const QColor colorS
) {
  QBitmap primary{pathP};
  QBitmap secondary{pathS};
  assert(primary.size() == secondary.size());
  primary = primary.scaled(primary.size() * glob_scale);
  secondary = secondary.scaled(secondary.size() * glob_scale);
  QPixmap pixmap{primary.size()};
  pixmap.fill(QColor{0, 0, 0, 0});
  const QRect rect = {QPoint{}, pixmap.size()};

  QPainter painter{&pixmap};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setClipRegion(primary);
  painter.fillRect(rect, colorP);
  painter.setClipRegion(secondary);
  painter.fillRect(rect, colorS);
  return pixmap;
}

}

constexpr HSV default_color = {0, 100, 100};
constexpr int default_alpha = 255;
inline const QColor border_color = glob_light_shade;
inline const QColor circle_primary_color = {0, 0, 0};
inline const QColor circle_secondary_color = {255, 255, 255};

class RectWidgetSize {
public:
  constexpr RectWidgetSize(const int padding, const int border, const QSize content)
    : padding{padding}, border{border}, content{content} {}

  constexpr QRect widget() const noexcept {
    return {
      0,
      0,
      content.width() + 2 * border + 2 * padding,
      content.height() + 2 * border + 2 * padding
    };
  }
  
  constexpr QRect outer() const noexcept {
    return {
      padding,
      padding,
      content.width() + 2 * border,
      content.height() + 2 * border
    };
  }
  
  constexpr QRect inner() const noexcept {
    return {
      padding + border,
      padding + border,
      content.width(),
      content.height()
    };
  }

//private:
  int padding;
  int border;
  QSize content;
};

constexpr RectWidgetSize svgraph_size = {
  1 * glob_scale,
  1 * glob_scale,
  {101 * glob_scale, 101 * glob_scale}
};

class SVGraph final : public QWidget {
  Q_OBJECT

public:
  explicit SVGraph(QWidget *parent)
    : QWidget{parent},
      graph{svgraph_size.inner().size(), QImage::Format_ARGB32_Premultiplied} {
    setFixedSize(svgraph_size.widget().size());
    initCircle();
    plotGraph(color.h);
    show();
  }

Q_SIGNALS:
  void svChanged(int, int);

public Q_SLOTS:
  void changeHue(const int hue) {
    color.h = hue;
    plotGraph(hue);
    repaint();
  }
  void changeSV(const int sat, const int val) {
    color.s = sat;
    color.v = val;
    repaint();
  }
  void changeHSV(const HSV hsv) {
    color = hsv;
    plotGraph(hsv.h);
    repaint();
  }

private:
  QImage graph;
  QPixmap circle;
  HSV color = default_color;
  bool mouseDown = false;
  
  void plotGraph(const int hue) {
    // x   - saturation
    // y   - value
    // 0,0 - bottom left
    
    QRgb *pixels = reinterpret_cast<QRgb *>(graph.bits());
    const ptrdiff_t pitch = graph.bytesPerLine() / sizeof(QRgb);
    const ptrdiff_t width = graph.width();
    const ptrdiff_t padding = pitch - width;
    int sat = 0;
    int val = svgraph_size.inner().height() - 1;
    
    QRgb *const imgEnd = pixels + pitch * graph.height();
    while (pixels != imgEnd) {
      QRgb *const rowEnd = pixels + width;
      while (pixels != rowEnd) {
        *pixels++ = hsv2rgb(
          hue, // can't use sat2pix and val2pix here
          sat++ * 100.0 / (svgraph_size.inner().width() - 1),
          val * 100.0 / (svgraph_size.inner().height() - 1)
        );
      }
      pixels += padding;
      --val;
      sat = 0;
    }
  }
  
  void initCircle() {
    circle = initColorBitmap(
      ":/Color Picker/sv circle p.pbm",
      ":/Color Picker/sv circle s.pbm",
      circle_primary_color,
      circle_secondary_color
    );
  }
  
  void renderGraph(QPainter &painter) {
    painter.drawImage(svgraph_size.inner(), graph);
  }
  
  void renderBorder(QPainter &painter) {
    painter.setBrush(border_color);
    painter.setPen(Qt::NoPen);
    ::renderBorder(painter, svgraph_size.inner(), svgraph_size.outer());
  }
  
  static int sat2pix(const int sat) {
    return qRound(sat / 100.0 * (svgraph_size.inner().width() - glob_scale));
  }
  static int val2pix(const int val) {
    return qRound((100 - val) / 100.0 * (svgraph_size.inner().height() - glob_scale));
  }
  static int pix2sat(const int pix) {
    return std::clamp(qRound(pix * 100.0 / (svgraph_size.inner().width() - glob_scale)), 0, 100);
  }
  static int pix2val(const int pix) {
    return 100 - std::clamp(qRound(pix * 100.0 / (svgraph_size.inner().height() - glob_scale)), 0, 100);
  }
  
  void renderCircle(QPainter &painter) {
    const QRect circleRect = {
      svgraph_size.inner().x() + sat2pix(color.s) - (circle.width() - glob_scale) / 2,
      svgraph_size.inner().y() + val2pix(color.v) - (circle.height() - glob_scale) / 2,
      circle.width(),
      circle.height()
    };
    painter.setClipRect(svgraph_size.inner());
    painter.drawPixmap(circleRect, circle);
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    renderGraph(painter);
    renderCircle(painter);
    renderBorder(painter);
  }
  
  void setColor(const QPointF point) {
    const int sat = pix2sat(point.x() - svgraph_size.inner().left());
    const int val = pix2val(point.y() - svgraph_size.inner().top());
    if (sat != color.s || val != color.v) {
      color.s = sat;
      color.v = val;
      repaint();
      Q_EMIT svChanged(sat, val);
    }
  }
  
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      mouseDown = true;
      setColor(event->localPos());
      grabMouse(Qt::BlankCursor);
    }
  }
  void mouseReleaseEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      mouseDown = false;
      setColor(event->localPos());
      releaseMouse();
    }
  }
  void mouseMoveEvent(QMouseEvent *event) override {
    if (mouseDown) {
      setColor(event->localPos());
    }
  }
};

constexpr RectWidgetSize slider_size = {
  svgraph_size.padding,
  svgraph_size.border,
  {svgraph_size.content.width(), 12 * glob_scale}
};

template <typename Derived>
class Slider : public QWidget {
public:
  explicit Slider(QWidget *parent)
    : QWidget{parent},
      graph{slider_size.inner().width(), 1, QImage::Format_ARGB32_Premultiplied} {
    setFixedSize(slider_size.widget().size());
    initBar();
    show();
  }

protected:
  QImage graph;

private:
  QPixmap bar;
  bool mouseDown = false;
  
  void initBar() {
    bar = initColorBitmap(
      ":/Color Picker/slider bar p.pbm",
      ":/Color Picker/slider bar s.pbm",
      circle_primary_color,
      circle_secondary_color
    );
  }
  
  void renderGraph(QPainter &painter) {
    painter.drawImage(slider_size.inner(), graph);
  }
  
  void renderBorder(QPainter &painter) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(border_color);
    ::renderBorder(painter, slider_size.inner(), slider_size.outer());
  }
  
  void renderBar(QPainter &painter) {
    const int pix = static_cast<Derived *>(this)->getPixel();
    const QRect barRect = {
      slider_size.inner().x() + pix - (bar.width() - glob_scale) / 2,
      0,
      bar.width(),
      bar.height()
    };
    painter.setClipRect(slider_size.widget());
    painter.drawPixmap(barRect, bar);
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    static_cast<Derived *>(this)->renderBackground(painter);
    renderGraph(painter);
    renderBorder(painter);
    renderBar(painter);
  }
  
  void setColor(QMouseEvent *event) {
    static_cast<Derived *>(this)->setColor(event->localPos().x() - slider_size.inner().x());
  }
  
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      mouseDown = true;
      setColor(event);
      grabMouse(Qt::BlankCursor);
    }
  }
  void mouseReleaseEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      mouseDown = false;
      setColor(event);
      releaseMouse();
    }
  }
  void mouseMoveEvent(QMouseEvent *event) override {
    if (mouseDown) {
      setColor(event);
    }
  }
};

class HueSlider final : public Slider<HueSlider> {
  Q_OBJECT
  
  friend class Slider<HueSlider>;

public:
  explicit HueSlider(QWidget *parent)
    : Slider{parent} {
    plotGraph();
  }

Q_SIGNALS:
  void hueChanged(int);

public Q_SLOTS:
  void changeHue(const int hue) {
    color.h = hue;
    repaint();
  }
  void changeSV(const int sat, const int val) {
    color.s = sat;
    color.v = val;
    plotGraph();
    repaint();
  }
  void changeHSV(const HSV hsv) {
    color = hsv;
    plotGraph();
    repaint();
  }
  
private:
  HSV color = default_color;
  
  void plotGraph() {
    // x - hue
    // 0 - left
    
    QRgb *pixels = reinterpret_cast<QRgb *>(graph.bits());
    QRgb *const pixelsEnd = pixels + graph.width();
    qreal hue = 0.0;
    int idx = 0;
    
    while (pixels != pixelsEnd) {
      *pixels++ = hsv2rgb(hue, color.s, color.v);
      // can't use hue2pix here
      hue = ++idx * 360.0 / slider_size.inner().width();
    }
  }
  
  void renderBackground(QPainter &) {}
  
  static int hue2pix(const int hue) {
    return qRound(hue / 359.0 * (slider_size.inner().width() - glob_scale));
  }
  static int pix2hue(const int pix) {
    return std::clamp(qRound(pix * 359.0 / (slider_size.inner().width() - glob_scale)), 0, 359);
  }
  
  int getPixel() {
    return hue2pix(color.h);
  }
  
  void setColor(const int pointX) {
    const int hue = pix2hue(pointX);
    if (hue != color.h) {
      color.h = hue;
      repaint();
      Q_EMIT hueChanged(hue);
    }
  }
};

constexpr int alpha_vert_tiles = 2;

class AlphaSlider final : public Slider<AlphaSlider> {
  Q_OBJECT
  
  friend class Slider<AlphaSlider>;
  
public:
  explicit AlphaSlider(QWidget *parent)
    : Slider{parent} {
    plotGraph();
  }
  
Q_SIGNALS:
  void alphaChanged(int);

public Q_SLOTS:
  void changeAlpha(const int newAlpha) {
    alpha = newAlpha;
    repaint();
  }
  void changeHue(const int hue) {
    color.h = hue;
    plotGraph();
    repaint();
  }
  void changeSV(const int sat, const int val) {
    color.s = sat;
    color.v = val;
    plotGraph();
    repaint();
  }
  void changeHSV(const HSV hsv) {
    color = hsv;
    plotGraph();
    repaint();
  }
  
private:
  HSV color = default_color;
  int alpha = default_alpha;
  
  static QRgb setAlpha(const QRgb color, const int alpha) {
    return qPremultiply(qRgba(qRed(color), qGreen(color), qBlue(color), alpha));
  }
  
  void plotGraph() {
    QRgb *pixels = reinterpret_cast<QRgb *>(graph.bits());
    QRgb *const pixelsEnd = pixels + graph.width();
    const QRgb rgb = hsv2rgb(color.h, color.s, color.v);
    int alp = 0;
    int idx = 0;
    
    while (pixels != pixelsEnd) {
      *pixels++ = setAlpha(rgb, alp);
      // can't use alp2pix here
      alp = qRound(++idx * 255.0 / (slider_size.inner().width() - 1));
    }
  }
  
  void renderBackground(QPainter &painter) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(edit_checker_a);
    painter.drawRect(slider_size.inner());
    painter.setBrush(edit_checker_b);
    painter.setClipRect(slider_size.inner());
    const int tileSize = slider_size.inner().height() / alpha_vert_tiles;
    const int horiTiles = slider_size.inner().width() / tileSize;
    for (int y = 0; y != alpha_vert_tiles; ++y) {
      for (int x = 1 - y; x <= horiTiles; x += 2) {
        painter.drawRect(
          slider_size.inner().x() + tileSize * x,
          slider_size.inner().y() + tileSize * y,
          tileSize,
          tileSize
        );
      }
    }
  }
  
  static int alp2pix(const int alp) {
    return qRound(alp / 255.0 * (slider_size.inner().width() - glob_scale));
  }
  static int pix2alp(const int pix) {
    return std::clamp(qRound(pix * 255.0 / (slider_size.inner().width() - glob_scale)), 0, 255);
  }
  
  int getPixel() {
    return alp2pix(alpha);
  }
  
  void setColor(const int pointX) {
    const int alp = pix2alp(pointX);
    if (alp != alpha) {
      alpha = alp;
      repaint();
      Q_EMIT alphaChanged(alpha);
    }
  }
};

constexpr int cursor_blink_interval_ms = 500;

class TextBox : public QLineEdit {
  Q_OBJECT
  
public:
  TextBox(QWidget *parent, const RectWidgetSize size, const int offsetX)
    : QLineEdit{parent},
      cursorBlinkTimer{this},
      boxSize{size},
      offsetX{offsetX} {
    setFixedSize(boxSize.widget().size());
    setFont(getGlobalFont());
    const int margin = boxSize.padding + boxSize.border;
    setTextMargins(margin + offsetX, margin, margin, margin);
    setFrame(false);
    setAttribute(Qt::WA_MacShowFocusRect, 0);
    
    cursorBlinkTimer.setInterval(cursor_blink_interval_ms);
    cursorBlinkTimer.setTimerType(Qt::CoarseTimer);
    connect(&cursorBlinkTimer, &QTimer::timeout, this, &TextBox::blinkSlot);
    cursorBlinkTimer.start();
    
    connect(this, &QLineEdit::selectionChanged, this, &TextBox::showCursor);
    connect(this, &QLineEdit::cursorPositionChanged, this, &TextBox::showCursor);
  }

private Q_SLOTS:
  void blinkSlot() {
    cursorBlinkStatus = !cursorBlinkStatus;
    repaint();
  }
  
  void showCursor() {
    cursorBlinkStatus = true;
    cursorBlinkTimer.stop();
    cursorBlinkTimer.start();
    repaint();
  }

private:
  QTimer cursorBlinkTimer;
  RectWidgetSize boxSize;
  int offsetX;
  bool cursorBlinkStatus = true;
  
  void focusInEvent(QFocusEvent *event) override {
    QLineEdit::focusInEvent(event);
    QTimer::singleShot(0, this, &QLineEdit::selectAll);
  }

  void renderBackground(QPainter &painter) {
    painter.setBrush(glob_dark_accent);
    painter.setPen(Qt::NoPen);
    painter.drawRect(boxSize.inner());
  }

  void renderBorder(QPainter &painter) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(border_color);
    ::renderBorder(painter, boxSize.inner(), boxSize.outer());
    painter.setClipRect(boxSize.inner());
  }

  void renderText(QPainter &painter) {
    painter.setFont(getGlobalFont());
    painter.setBrush(Qt::NoBrush);
    painter.setPen(glob_light_shade);
    QPoint textPos = boxSize.inner().topLeft();
    textPos += QPoint{offsetX, glob_font_accent_px};
    textPos += QPoint{1 * glob_scale, 1 * glob_scale};
    painter.drawText(textPos, text());
  }
  
  void renderCursor(QPainter &painter) {
    if (!hasFocus() || !cursorBlinkStatus || selectionStart() != -1) return;
    painter.setBrush(glob_light_shade);
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRect{
      cursorPosition() * glob_scale * 6 + 4 + offsetX, 4,
      2, glob_font_accent_px + 4
    });
  }

  void renderSelection(QPainter &painter) {
    if (!hasFocus() || selectionStart() == -1) return;
    QColor color = glob_light_accent;
    color.setAlpha(127);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRect{
      selectionStart() * glob_scale * 6 + 4 + offsetX,
      4,
      selectionLength() * glob_scale * 6 + 2,
      glob_font_accent_px + 4
    });
  }

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    renderBackground(painter);
    renderBorder(painter);
    renderText(painter);
    renderCursor(painter);
    renderSelection(painter);
  }
};

constexpr RectWidgetSize number_box_size = {
  svgraph_size.padding,
  svgraph_size.border,
  {((5 + 1) * 3 + 1) * glob_scale, glob_font_px + 2 * glob_scale}
};
constexpr RectWidgetSize hex_box_size = {
  svgraph_size.padding,
  svgraph_size.border,
  {((5 + 1) * 8 + 1 + 5) * glob_scale, glob_font_px + 2 * glob_scale}
};

class NumberBoxValidator final : public QIntValidator {
public:
  NumberBoxValidator(QWidget *parent, const int max)
    : QIntValidator{0, max, parent} {}
  
  void fixup(QString &input) const override {
    input = lastValidValue;
  }
  
  void updateValidValue(const QString &value) {
    lastValidValue = value;
  }

private:
  QString lastValidValue;
};

class HexBoxValidator final : public QValidator {
public:
  explicit HexBoxValidator(QWidget *parent)
    : QValidator{parent} {}
  
  void fixup(QString &input) const override {
    if (input.size() == 6) {
      input.append('F');
      input.append('F');
    } else {
      input = lastValidValue;
    }
  }
  
  State validate(QString &input, int &pos) const override {
    if (input.front() == '#') {
      input.remove(0, 1);
      pos = std::max(pos - 1, 0);
    }
    if (input.size() > 8) return State::Invalid;
    for (QChar &ch : input) {
      const char latin1 = ch.toLatin1();
      if ('0' <= latin1 && latin1 <= '9') continue;
      if ('A' <= latin1 && latin1 <= 'F') continue;
      if ('a' <= latin1 && latin1 <= 'f') {
        ch = ch.toUpper();
        continue;
      }
      return State::Invalid;
    }
    if (input.size() == 8) {
      return State::Acceptable;
    } else {
      return State::Intermediate;
    }
  }
  
  void updateValidValue(const QString &value) {
    lastValidValue = value;
  }

private:
  QString lastValidValue;
};

class NumberBox final : public TextBox {
  Q_OBJECT
public:
  NumberBox(QWidget *parent, const int defaultValue, const int max)
    : TextBox{parent, number_box_size, 0}, boxValidator{parent, max} {
    setValidator(&boxValidator);
    changeValue(defaultValue);
    connect(this, &QLineEdit::textEdited, this, &NumberBox::textChanged);
    connect(this, &QLineEdit::editingFinished, this, &NumberBox::newValidValue);
  }

Q_SIGNALS:
  void valueChanged(int);

public Q_SLOTS:
  void changeValue(const int num) {
    value = num;
    setText(QString::number(value));
    boxValidator.updateValidValue(text());
  }

private Q_SLOTS:
  void textChanged() {
    if (!hasAcceptableInput()) return;
    const int newValue = text().toInt();
    if (value != newValue) {
      value = newValue;
      Q_EMIT valueChanged(value);
    }
  }
  
  void newValidValue() {
    if (hasAcceptableInput()) {
      boxValidator.updateValidValue(text());
    }
  }

private:
  NumberBoxValidator boxValidator;
  int value;
};

class HexBox final : public TextBox {
  Q_OBJECT
public:
  HexBox(QWidget *parent, const QRgb defaultValue)
    : TextBox{parent, hex_box_size, 2 * glob_scale}, boxValidator{parent} {
    setValidator(&boxValidator);
    changeRgba(defaultValue);
    connect(this, &QLineEdit::textEdited, this, &HexBox::textChanged);
    connect(this, &QLineEdit::editingFinished, this, &HexBox::newValidValue);
  }

Q_SIGNALS:
  void rgbaChanged(QRgb);

public Q_SLOTS:
  void changeRgba(const QRgb color) {
    value = color;
    setText(toString(value));
    boxValidator.updateValidValue(text());
  }

private Q_SLOTS:
  void textChnaged() {
    if (!hasAcceptableInput()) return;
    const QRgb newValue = fromString(text());
    if (value != newValue) {
      value = newValue;
      Q_EMIT rgbaChanged(value);
    }
  }
  
  void newValidValue() {
    if (hasAcceptableInput()) {
      boxValidator.updateValidValue(text());
    }
  }

private:
  HexBoxValidator boxValidator;
  QRgb value;
  
  QString toString(const QRgb color) {
    const uint number = (qRed(color)   << 24) |
                        (qGreen(color) << 16) |
                        (qBlue(color)  <<  8) |
                         qAlpha(color);
    QString str = QString::number(number, 16);
    while (str.size() < 8) {
      str.prepend("0");
    }
    return str;
  }
  QRgb fromString(const QString &string) {
    uint number = string.toUInt(nullptr, 16);
    return qRgba(
       number >> 24,
      (number >> 16) & 255,
      (number >>  8) & 255,
       number        & 255
    );
  }
};

constexpr RectWidgetSize label_size = {
  svgraph_size.padding,
  svgraph_size.border,
  {((5 + 1) * 1 + 2) * glob_scale, glob_font_px + 2 * glob_scale}
};

class BoxLabel final : public QWidget {
public:
  BoxLabel(QWidget *parent, const QString &text)
    : QWidget{parent}, text{text} {
    setFixedSize(label_size.widget().size());
  }

private:
  QString text;

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.setFont(getGlobalFont());
    painter.setBrush(Qt::NoBrush);
    painter.setPen(glob_light_shade);
    QPoint textPos = label_size.inner().topLeft();
    textPos.ry() += glob_font_accent_px;
    textPos += QPoint{2 * glob_scale, 1 * glob_scale};
    painter.drawText(textPos, text);
  }
};

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
  : QWidget{parent},
    svGraph{new SVGraph{this}},
    hueSlider{new HueSlider{this}},
    alphaSlider{new AlphaSlider{this}},
    boxR{new NumberBox{this, 89, 255}},
    boxG{new NumberBox{this, 89, 255}},
    boxB{new NumberBox{this, 89, 255}},
    boxH{new NumberBox{this, default_color.h, 359}},
    boxS{new NumberBox{this, 89, 100}},
    boxV{new NumberBox{this, 89, 100}},
    boxA{new NumberBox{this, default_alpha, 255}},
    boxHex{new HexBox{this, 0x89898989}},
    labelR{new BoxLabel{this, "R"}},
    labelG{new BoxLabel{this, "G"}},
    labelB{new BoxLabel{this, "B"}},
    labelH{new BoxLabel{this, "H"}},
    labelS{new BoxLabel{this, "S"}},
    labelV{new BoxLabel{this, "V"}},
    labelA{new BoxLabel{this, "A"}},
    labelHex{new BoxLabel{this, "#"}} {
  setFixedWidth(svgraph_size.widget().width());
  setupLayout();
  connectSignals();
  show();
}

void ColorPickerWidget::setupLayout() {
  QGridLayout *layout = new QGridLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  
  layout->addWidget(svGraph, 0, 0, 1, 6);
  layout->addWidget(hueSlider, 1, 0, 1, 6);
  layout->addWidget(alphaSlider, 2, 0, 1, 6);
  
  layout->addWidget(labelR, 3, 0);
  layout->addWidget(boxR,   3, 1);
  layout->addWidget(labelG, 3, 2);
  layout->addWidget(boxG,   3, 3);
  layout->addWidget(labelB, 3, 4);
  layout->addWidget(boxB,   3, 5);
  
  layout->addWidget(labelH, 4, 0);
  layout->addWidget(boxH,   4, 1);
  layout->addWidget(labelS, 4, 2);
  layout->addWidget(boxS,   4, 3);
  layout->addWidget(labelV, 4, 4);
  layout->addWidget(boxV,   4, 5);
  
  layout->addWidget(labelA,   5, 0);
  layout->addWidget(boxA,     5, 1);
  layout->addWidget(labelHex, 5, 2);
  layout->addWidget(boxHex,   5, 3, 1, 3);
  
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);
}

void ColorPickerWidget::connectSignals() {
  connect(svGraph, &SVGraph::svChanged, hueSlider, &HueSlider::changeSV);
  connect(hueSlider, &HueSlider::hueChanged, svGraph, &SVGraph::changeHue);
  connect(hueSlider, &HueSlider::hueChanged, alphaSlider, &AlphaSlider::changeHue);
  connect(svGraph, &SVGraph::svChanged, alphaSlider, &AlphaSlider::changeSV);
  
  connect(alphaSlider, &AlphaSlider::alphaChanged, boxA, &NumberBox::changeValue);
  connect(boxA, &NumberBox::valueChanged, alphaSlider, &AlphaSlider::changeAlpha);
  
  connect(hueSlider, &HueSlider::hueChanged, boxH, &NumberBox::changeValue);
  connect(boxH, &NumberBox::valueChanged, hueSlider, &HueSlider::changeHue);
  
  // create an object that has a bunch of signals and slots for converting
  // between hsv and rgb
  
  // maybe it could be the ColorPickerWidget?
}

#include "color picker widget.moc"

