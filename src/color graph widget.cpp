//
//  color graph widget.cpp
//  Animera
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color graph widget.hpp"

#include "config.hpp"
#include "status msg.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include "widget painting.hpp"
#include "surface factory.hpp"
#include <Graphics/iterator.hpp>

SVGraphWidget::SVGraphWidget(QWidget *parent, const int alpha)
  : QWidget{parent},
    graph{pick_svgraph_rect.inner().size(), QImage::Format_ARGB32_Premultiplied},
    color{color2hsv(pick_default_color)},
    alpha{alpha} {
  setMouseTracking(true);
  setFocusPolicy(Qt::ClickFocus);
  setFixedSize(pick_svgraph_rect.widget().size());
  initCircle();
  plotGraph(color.h);
  show();
}

void SVGraphWidget::setHue(const int hue) {
  color.h = hue;
  plotGraph(hue);
  repaint();
}

void SVGraphWidget::setSV(const int sat, const int val) {
  color.s = sat;
  color.v = val;
  repaint();
}

void SVGraphWidget::setHSV(const HSV hsv) {
  color = hsv;
  plotGraph(hsv.h);
  repaint();
}

void SVGraphWidget::setAlpha(const int alp) {
  alpha = alp;
}

void SVGraphWidget::plotGraph(const int hue) {
  // x   - saturation
  // y   - value
  // 0,0 - bottom left
  
  int sat = 0;
  int val = pick_svgraph_rect.inner().height() - 1;
  for (auto row : gfx::range(makeSurface<QRgb>(graph))) {
    for (QRgb &pixel : row) {
      pixel = hsv2rgb(
        hue, // can't use pix2sat and pix2val here
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
    ":/Color Picker/graph circle p.png",
    ":/Color Picker/graph circle s.png",
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

void SVGraphWidget::updateSV(const int sat, const int val) {
  if (sat != color.s || val != color.v) {
    color.s = sat;
    color.v = val;
    repaint();
    Q_EMIT svChanged(sat, val);
  }
}

void SVGraphWidget::setColor(QPointF point) {
  point -= pick_svgraph_rect.inner().topLeft();
  updateSV(pix2sat(point.x()), pix2val(point.y()));
}

void SVGraphWidget::updateStatus(QPointF point) {
  point -= pick_svgraph_rect.inner().topLeft();
  const int sat = pix2sat(point.x());
  const int val = pix2val(point.y());
  StatusMsg status;
  status.append("SATURATION: ");
  status.append(sat);
  status.append(" VALUE: ");
  status.append(val);
  status.append(" COLOR: ");
  const RGB rgbColor = hsv2rgb({color.h, sat, val});
  status.append(rgbColor.r, rgbColor.g, rgbColor.b, alpha);
  Q_EMIT shouldShowNorm(status.get());
}

void SVGraphWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mouseDown = true;
    setColor(event->localPos());
  }
}

void SVGraphWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mouseDown = false;
    setColor(event->localPos());
  }
}

void SVGraphWidget::mouseMoveEvent(QMouseEvent *event) {
  if (mouseDown) {
    setColor(event->localPos());
  }
  updateStatus(event->localPos());
}

void SVGraphWidget::leaveEvent(QEvent *) {
  Q_EMIT shouldShowNorm("");
}

void SVGraphWidget::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
    case Qt::Key_Up:
      return updateSV(color.s, std::min(color.v + 1, 100));
    case Qt::Key_Right:
      return updateSV(std::min(color.s + 1, 100), color.v);
    case Qt::Key_Down:
      return updateSV(color.s, std::max(color.v - 1, 0));
    case Qt::Key_Left:
      return updateSV(std::max(color.s - 1, 0), color.v);
  }
}

#include "color graph widget.moc"
