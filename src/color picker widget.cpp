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
  void changeSat(const int sat) {
    color.s = sat;
    repaint();
  }
  void changeVal(const int val) {
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
          hue,
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
  
  void renderCircle(QPainter &painter) {
    const QRect circleRect = {
      svgraph_inner_rect.x() + color.s * glob_scale - (circle.width() - glob_scale) / 2,
      svgraph_inner_rect.y() + (100 - color.v) * glob_scale - (circle.height() - glob_scale) / 2,
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
  
  void setColor(QPointF point) {
    point -= QPointF{svgraph_inner_rect.topLeft()};
    point /= glob_scale;
    const int sat = std::clamp(qRound(point.x()), 0, 100);
    const int val = 100 - std::clamp(qRound(point.y()), 0, 100);
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
      grabMouse();
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

class HueSlider final : public QWidget {
  Q_OBJECT

public:
  explicit HueSlider(QWidget *parent)
    : QWidget{parent},
      graph{{slider_inner_rect.width(), 1}, QImage::Format_ARGB32_Premultiplied} {
    setFixedSize(slider_widget_rect.size());
    initBar();
    plotGraph(color.s, color.v);
    show();
  }

Q_SIGNALS:
  void hueChanged(int);

public Q_SLOTS:
  void changeHue(const int hue) {
    color.h = hue;
    repaint();
  }
  void changeSat(const int sat) {
    color.s = sat;
    plotGraph(sat, color.v);
    repaint();
  }
  void changeVal(const int val) {
    color.v = val;
    plotGraph(color.s, val);
    repaint();
  }
  void changeSV(const int sat, const int val) {
    color.s = sat;
    color.v = val;
    plotGraph(sat, val);
    repaint();
  }
  void changeHSV(const HSV hsv) {
    color = hsv;
    plotGraph(hsv.s, hsv.v);
    repaint();
  }
  
private:
  QImage graph;
  QPixmap bar;
  HSV color = default_color;
  bool mouseDown = false;
  
  void plotGraph(const int sat, const int val) {
    // x - hue
    // 0 - left
    
    QRgb *pixels = reinterpret_cast<QRgb *>(graph.bits());
    const ptrdiff_t width = graph.width();
    qreal hue = 0.0;
    int idx = 0;
    
    QRgb *const imgEnd = pixels + width;
    while (pixels != imgEnd) {
      *pixels++ = hsv2rgb(hue, sat, val);
      hue = ++idx * 360.0 / slider_inner_rect.width();
    }
  }
  
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
  
  static int hue2pix(const int hue) {
    return qRound(hue / 360.0 * slider_inner_rect.width());
  }
  
  static int pix2hue(const int pix) {
    return std::clamp(qRound(pix * 360.0 / slider_inner_rect.width()), 0, 359);
  }
  
  void renderBar(QPainter &painter) {
    const QRect barRect = {
      slider_inner_rect.x() + hue2pix(color.h) - (bar.width() - glob_scale) / 2,
      0,
      bar.width(),
      bar.height()
    };
    painter.setClipRegion(slider_widget_rect);
    painter.drawPixmap(barRect, bar);
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    renderGraph(painter);
    renderBorder(painter);
    renderBar(painter);
  }
  
  void setColor(const int pointX) {
    const int hue = pix2hue(pointX - slider_inner_rect.x());
    if (hue != color.h) {
      color.h = hue;
      repaint();
      Q_EMIT hueChanged(hue);
    }
  }
  
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      mouseDown = true;
      setColor(event->localPos().x());
      grabMouse();
    }
  }
  void mouseReleaseEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      mouseDown = false;
      setColor(event->localPos().x());
      releaseMouse();
    }
  }
  void mouseMoveEvent(QMouseEvent *event) override {
    if (mouseDown) {
      setColor(event->localPos().x());
    }
  }
};

class AlphaSlider final : public QWidget {
public:
  explicit AlphaSlider(QWidget *parent)
    : QWidget{parent} {}
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
  layout->addStretch();
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);
}

void ColorPickerWidget::connectSignals() {
  connect(svGraph, &SVGraph::svChanged, hueSlider, &HueSlider::changeSV);
  connect(hueSlider, &HueSlider::hueChanged, svGraph, &SVGraph::changeHue);
}

#include "color picker widget.moc"
