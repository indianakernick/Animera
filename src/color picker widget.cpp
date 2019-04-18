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

constexpr int graph_pixels = 101;
constexpr int graph_size = graph_pixels * glob_scale;
constexpr int border_size = 1 * glob_scale;
constexpr int widget_size = graph_size + 2 * border_size;
inline const QColor border_color = glob_light_shade;
inline const QColor circle_primary_color = {0, 0, 0};
inline const QColor circle_secondary_color = {255, 255, 255};

struct HSV {
  int h, s, v;
};

class SVGraph final : public QWidget {
  Q_OBJECT

public:
  explicit SVGraph(QWidget *parent)
    : QWidget{parent},
      graph{{graph_pixels, graph_pixels}, QImage::Format_ARGB32_Premultiplied} {
    setFixedSize(widget_size, widget_size);
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
  HSV color = {0, 100, 100};
  bool mouseDown = false;
  
  static QRgb hsv2rgb(const int h, const int s, const int v) {
    QColor color;
    color.setHsvF(static_cast<qreal>(h), s / 100.0, v / 100.0);
    return color.rgba();
  }
  
  void plotGraph(const int hue) {
    // x   - saturation
    // y   - value
    // 0,0 - bottom left
    
    QRgb *pixels = reinterpret_cast<QRgb *>(graph.bits());
    const ptrdiff_t pitch = graph.bytesPerLine() / sizeof(QRgb);
    const ptrdiff_t width = graph.width();
    const ptrdiff_t padding = pitch - width;
    int sat = 0;
    int val = 100;
    
    QRgb *const imgEnd = pixels + pitch * graph.height();
    while (pixels != imgEnd) {
      QRgb *const rowEnd = pixels + width;
      while (pixels != rowEnd) {
        *pixels++ = hsv2rgb(hue, sat++, val);
      }
      pixels += padding;
      --val;
      sat = 0;
    }
  }
  
  void initCircle() {
    QBitmap primary{":/Color Picker/sv circle p.pbm"};
    QBitmap secondary{":/Color Picker/sv circle s.pbm"};
    assert(primary.size() == secondary.size());
    primary = primary.scaled(primary.size() * glob_scale);
    secondary = secondary.scaled(secondary.size() * glob_scale);
    circle = QPixmap{primary.size()};
    circle.fill(QColor{0, 0, 0, 0});
    const QRect circleRect = {QPoint{}, circle.size()};
    
    QPainter painter{&circle};
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setClipRegion(primary);
    painter.fillRect(circleRect, circle_primary_color);
    painter.setClipRegion(secondary);
    painter.fillRect(circleRect, circle_secondary_color);
  }
  
  void renderGraph(QPainter &painter) {
    const QRect graphRect = {border_size, border_size, graph_size, graph_size};
    painter.drawImage(graphRect, graph);
  }
  
  void renderBorder(QPainter &painter) {
    const QRect border[] = {
      {0, 0, widget_size, border_size}, // top
      {0, border_size, border_size, graph_size}, // left
      {0, widget_size - border_size, widget_size, border_size}, // bottom
      {widget_size - border_size, border_size, border_size, graph_size}, // right
    };
    painter.setBrush(border_color);
    painter.setPen(Qt::NoPen);
    painter.drawRects(border, std::size(border));
  }
  
  void renderCircle(QPainter &painter) {
    const QRect circleRect = {
      border_size + color.s * glob_scale - (circle.width() - glob_scale) / 2,
      border_size + (100 - color.v) * glob_scale - (circle.height() - glob_scale) / 2,
      circle.width(),
      circle.height()
    };
    painter.drawPixmap(circleRect, circle);
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    renderGraph(painter);
    renderCircle(painter);
    renderBorder(painter);
  }
  
  void setColor(QPointF point) {
    point -= QPointF{border_size, border_size};
    point /= glob_scale;
    const int sat = std::clamp(qRound(point.x()), 0, 100);
    const int val = 100 - std::clamp(qRound(point.y()), 0, 100);
    if (sat != color.s != val != color.v) {
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

class HueSlider final : public QWidget {
public:
  explicit HueSlider(QWidget *parent)
    : QWidget{parent} {}
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
  setFixedWidth(widget_size);
  setMinimumHeight(widget_size);
  show();
}

#include "color picker widget.moc"
