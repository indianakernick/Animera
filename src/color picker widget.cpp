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
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qboxlayout.h>

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

struct RectWidgetSize {
  int padding;
  int border;
  QSize content;
};

constexpr QRect makeWidgetRect(const RectWidgetSize spec) {
  return {
    0,
    0,
    spec.content.width() + 2 * spec.border + 2 * spec.padding,
    spec.content.height() + 2 * spec.border + 2 * spec.padding
  };
}

constexpr QRect makeOuterRect(const RectWidgetSize spec) {
  return {
    spec.padding,
    spec.padding,
    spec.content.width() + 2 * spec.border,
    spec.content.height() + 2 * spec.border
  };
}

constexpr QRect makeInnerRect(const RectWidgetSize spec) {
  return {
    spec.padding + spec.border,
    spec.padding + spec.border,
    spec.content.width(),
    spec.content.height()
  };
}

constexpr RectWidgetSize svgraph_size = {
  1 * glob_scale,
  1 * glob_scale,
  {101 * glob_scale, 101 * glob_scale}
};
constexpr QRect svgraph_widget_rect = makeWidgetRect(svgraph_size);
constexpr QRect svgraph_outer_rect = makeOuterRect(svgraph_size);
constexpr QRect svgraph_inner_rect = makeInnerRect(svgraph_size);

class SVGraph final : public QWidget {
  Q_OBJECT

public:
  explicit SVGraph(QWidget *parent)
    : QWidget{parent},
      graph{svgraph_inner_rect.size(), QImage::Format_ARGB32_Premultiplied} {
    setFixedSize(svgraph_widget_rect.size());
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
    int val = svgraph_inner_rect.height() - 1;
    
    QRgb *const imgEnd = pixels + pitch * graph.height();
    while (pixels != imgEnd) {
      QRgb *const rowEnd = pixels + width;
      while (pixels != rowEnd) {
        *pixels++ = hsv2rgb(
          hue, // can't use sat2pix and val2pix here
          sat++ * 100.0 / (svgraph_inner_rect.width() - 1),
          val * 100.0 / (svgraph_inner_rect.height() - 1)
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
    painter.drawImage(svgraph_inner_rect, graph);
  }
  
  void renderBorder(QPainter &painter) {
    painter.setBrush(border_color);
    painter.setPen(Qt::NoPen);
    ::renderBorder(painter, svgraph_inner_rect, svgraph_outer_rect);
  }
  
  static int sat2pix(const int sat) {
    return qRound(sat / 100.0 * (svgraph_inner_rect.width() - glob_scale));
  }
  static int val2pix(const int val) {
    return qRound((100 - val) / 100.0 * (svgraph_inner_rect.height() - glob_scale));
  }
  static int pix2sat(const int pix) {
    return std::clamp(qRound(pix * 100.0 / (svgraph_inner_rect.width() - glob_scale)), 0, 100);
  }
  static int pix2val(const int pix) {
    return 100 - std::clamp(qRound(pix * 100.0 / (svgraph_inner_rect.height() - glob_scale)), 0, 100);
  }
  
  void renderCircle(QPainter &painter) {
    const QRect circleRect = {
      svgraph_inner_rect.x() + sat2pix(color.s) - (circle.width() - glob_scale) / 2,
      svgraph_inner_rect.y() + val2pix(color.v) - (circle.height() - glob_scale) / 2,
      circle.width(),
      circle.height()
    };
    painter.setClipRegion(svgraph_inner_rect);
    painter.drawPixmap(circleRect, circle);
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    renderGraph(painter);
    renderCircle(painter);
    renderBorder(painter);
  }
  
  void setColor(const QPointF point) {
    const int sat = pix2sat(point.x() - svgraph_inner_rect.left());
    const int val = pix2val(point.y() - svgraph_inner_rect.top());
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
constexpr QRect slider_widget_rect = makeWidgetRect(slider_size);
constexpr QRect slider_outer_rect = makeOuterRect(slider_size);
constexpr QRect slider_inner_rect = makeInnerRect(slider_size);

template <typename Derived>
class Slider : public QWidget {
public:
  explicit Slider(QWidget *parent)
    : QWidget{parent},
      graph{slider_inner_rect.width(), 1, QImage::Format_ARGB32_Premultiplied} {
    setFixedSize(slider_widget_rect.size());
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
    painter.drawImage(slider_inner_rect, graph);
  }
  
  void renderBorder(QPainter &painter) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(border_color);
    ::renderBorder(painter, slider_inner_rect, slider_outer_rect);
  }
  
  void renderBar(QPainter &painter) {
    const int pix = static_cast<Derived *>(this)->getPixel();
    const QRect barRect = {
      slider_inner_rect.x() + pix - (bar.width() - glob_scale) / 2,
      0,
      bar.width(),
      bar.height()
    };
    painter.setClipRegion(slider_widget_rect);
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
    static_cast<Derived *>(this)->setColor(event->localPos().x() - slider_inner_rect.x());
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
      hue = ++idx * 360.0 / slider_inner_rect.width();
    }
  }
  
  void renderBackground(QPainter &) {}
  
  static int hue2pix(const int hue) {
    return qRound(hue / 359.0 * (slider_inner_rect.width() - glob_scale));
  }
  static int pix2hue(const int pix) {
    return std::clamp(qRound(pix * 359.0 / (slider_inner_rect.width() - glob_scale)), 0, 359);
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
      alp = qRound(++idx * 255.0 / (slider_inner_rect.width() - 1));
    }
  }
  
  void renderBackground(QPainter &painter) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(edit_checker_a);
    painter.drawRect(slider_inner_rect);
    painter.setBrush(edit_checker_b);
    painter.setClipRegion(slider_inner_rect);
    const int tileSize = slider_inner_rect.height() / alpha_vert_tiles;
    const int horiTiles = slider_inner_rect.width() / tileSize;
    for (int y = 0; y != alpha_vert_tiles; ++y) {
      for (int x = 1 - y; x <= horiTiles; x += 2) {
        painter.drawRect(
          slider_inner_rect.x() + tileSize * x,
          slider_inner_rect.y() + tileSize * y,
          tileSize,
          tileSize
        );
      }
    }
  }
  
  static int alp2pix(const int alp) {
    return qRound(alp / 255.0 * (slider_inner_rect.width() - glob_scale));
  }
  static int pix2alp(const int pix) {
    return std::clamp(qRound(pix * 255.0 / (slider_inner_rect.width() - glob_scale)), 0, 255);
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

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
  : QWidget{parent},
    svGraph{new SVGraph{this}},
    hueSlider{new HueSlider{this}},
    alphaSlider{new AlphaSlider{this}} {
  setFixedWidth(svgraph_widget_rect.width());
  setupLayout();
  connectSignals();
  show();
}

void ColorPickerWidget::setupLayout() {
  QVBoxLayout *layout = new QVBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(svGraph);
  layout->addWidget(hueSlider);
  layout->addWidget(alphaSlider);
  layout->addStretch();
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);
}

void ColorPickerWidget::connectSignals() {
  connect(svGraph, &SVGraph::svChanged, hueSlider, &HueSlider::changeSV);
  connect(hueSlider, &HueSlider::hueChanged, svGraph, &SVGraph::changeHue);
  connect(hueSlider, &HueSlider::hueChanged, alphaSlider, &AlphaSlider::changeHue);
  connect(svGraph, &SVGraph::svChanged, alphaSlider, &AlphaSlider::changeSV);
}

#include "color picker widget.moc"
