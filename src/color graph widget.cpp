//
//  color graph widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color graph widget.hpp"

#include "config.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include "widget painting.hpp"
#include "surface factory.hpp"

SVGraphWidget::SVGraphWidget(QWidget *parent)
  : QWidget{parent},
    graph{pick_svgraph_rect.inner().size(), QImage::Format_ARGB32_Premultiplied},
    color{color2hsv(pick_default_color)} {
  setFixedSize(pick_svgraph_rect.widget().size());
  initCircle();
  plotGraph(color.h);
  show();
}

void SVGraphWidget::changeHue(const int hue) {
  color.h = hue;
  plotGraph(hue);
  repaint();
}

void SVGraphWidget::changeSV(const int sat, const int val) {
  color.s = sat;
  color.v = val;
  repaint();
}

void SVGraphWidget::changeHSV(const HSV hsv) {
  color = hsv;
  plotGraph(hsv.h);
  repaint();
}

void SVGraphWidget::plotGraph(const int hue) {
  // x   - saturation
  // y   - value
  // 0,0 - bottom left
  
  int sat = 0;
  int val = pick_svgraph_rect.inner().height() - 1;
  for (auto row : makeSurface<QRgb>(graph).range()) {
    for (QRgb &pixel : row) {
      pixel = hsv2rgb(
        hue, // can't use sat2pix and val2pix here
        sat++ * 100.0 / (pick_svgraph_rect.inner().width() - 1),
        val * 100.0 / (pick_svgraph_rect.inner().height() - 1)
      );
    }
    --val;
    sat = 0;
  }
}

void SVGraphWidget::initCircle() {
  circle = bakeColoredBitmaps(
    ":/Color Picker/sv circle p.pbm",
    ":/Color Picker/sv circle s.pbm",
    pick_primary_color,
    pick_secondary_color
  );
}

void SVGraphWidget::renderGraph(QPainter &painter) {
  painter.drawImage(pick_svgraph_rect.inner(), graph);
}

namespace {

int sat2pix(const int sat) {
  return qRound(sat / 100.0 * (pick_svgraph_rect.inner().width() - 1_px));
}

int val2pix(const int val) {
  return qRound((100 - val) / 100.0 * (pick_svgraph_rect.inner().height() - 1_px));
}

int pix2sat(const int pix) {
  return std::clamp(qRound(pix * 100.0 / (pick_svgraph_rect.inner().width() - 1_px)), 0, 100);
}

int pix2val(const int pix) {
  return 100 - std::clamp(qRound(pix * 100.0 / (pick_svgraph_rect.inner().height() - 1_px)), 0, 100);
}

}

void SVGraphWidget::renderCircle(QPainter &painter) {
  const QRect circleRect = {
    pick_svgraph_rect.inner().x() + sat2pix(color.s) - (circle.width() - 1_px) / 2,
    pick_svgraph_rect.inner().y() + val2pix(color.v) - (circle.height() - 1_px) / 2,
    circle.width(),
    circle.height()
  };
  painter.setClipRect(pick_svgraph_rect.inner());
  painter.drawPixmap(circleRect, circle);
  painter.setClipRect(pick_svgraph_rect.widget());
}

void SVGraphWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  renderGraph(painter);
  renderCircle(painter);
  paintBorder(painter, pick_svgraph_rect, glob_border_color);
}

void SVGraphWidget::setColor(const QPointF point) {
  const int sat = pix2sat(point.x() - pick_svgraph_rect.inner().x());
  const int val = pix2val(point.y() - pick_svgraph_rect.inner().y());
  if (sat != color.s || val != color.v) {
    color.s = sat;
    color.v = val;
    repaint();
    Q_EMIT svChanged(sat, val);
  }
}

void SVGraphWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mouseDown = true;
    setColor(event->localPos());
    grabMouse(Qt::BlankCursor);
  }
}

void SVGraphWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mouseDown = false;
    setColor(event->localPos());
    releaseMouse();
  }
}

void SVGraphWidget::mouseMoveEvent(QMouseEvent *event) {
  if (mouseDown) {
    setColor(event->localPos());
  }
}

#include "color graph widget.moc"
