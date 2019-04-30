//
//  color slider widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color slider widget.hpp"

#include "config.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include "widget painting.hpp"

template <typename Derived>
ColorSliderWidget<Derived>::ColorSliderWidget(QWidget *parent)
  : QWidget{parent},
    graph{pick_slider_rect.inner().width(), 1, QImage::Format_ARGB32_Premultiplied} {
  setFixedSize(pick_slider_rect.widget().size());
  initBar();
  show();
}

template <typename Derived>
void ColorSliderWidget<Derived>::initBar() {
  bar = bakeColoredBitmaps(
    ":/Color Picker/slider bar p.pbm",
    ":/Color Picker/slider bar s.pbm",
    pick_primary_color,
    pick_secondary_color
  );
}

template <typename Derived>
void ColorSliderWidget<Derived>::renderGraph(QPainter &painter) {
  painter.drawImage(pick_slider_rect.inner(), graph);
}

template <typename Derived>
void ColorSliderWidget<Derived>::renderBar(QPainter &painter) {
  const int pix = static_cast<Derived *>(this)->getPixel();
  const QRect barRect = {
    pick_slider_rect.inner().x() + pix - (bar.width() - 1_px) / 2,
    0,
    bar.width(),
    bar.height()
  };
  painter.drawPixmap(barRect, bar);
}

template <typename Derived>
void ColorSliderWidget<Derived>::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  static_cast<Derived *>(this)->renderBackground(painter);
  renderGraph(painter);
  paintBorder(painter, pick_slider_rect, glob_border_color);
  renderBar(painter);
}

template <typename Derived>
void ColorSliderWidget<Derived>::setColor(QMouseEvent *event) {
  static_cast<Derived *>(this)->setColor(event->localPos().x() - pick_slider_rect.inner().x());
}

template <typename Derived>
void ColorSliderWidget<Derived>::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mouseDown = true;
    setColor(event);
    grabMouse(Qt::BlankCursor);
  }
}

template <typename Derived>
void ColorSliderWidget<Derived>::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mouseDown = false;
    setColor(event);
    releaseMouse();
  }
}

template <typename Derived>
void ColorSliderWidget<Derived>::mouseMoveEvent(QMouseEvent *event) {
  if (mouseDown) {
    setColor(event);
  }
}

HueSliderWidget::HueSliderWidget(QWidget *parent)
  : ColorSliderWidget{parent},
    color{color2hsv(pick_default_color)} {
  plotGraph();
}

void HueSliderWidget::changeHue(const int hue) {
  color.h = hue;
  repaint();
}

void HueSliderWidget::changeSV(const int sat, const int val) {
  color.s = sat;
  color.v = val;
  plotGraph();
  repaint();
}

void HueSliderWidget::changeHSV(const HSV hsv) {
  color = hsv;
  plotGraph();
  repaint();
}

void HueSliderWidget::plotGraph() {
  // x - hue
  // 0 - left
  
  QRgb *pixels = reinterpret_cast<QRgb *>(graph.bits());
  QRgb *const pixelsEnd = pixels + graph.width();
  qreal hue = 0.0;
  int idx = 0;
  
  while (pixels != pixelsEnd) {
    *pixels++ = hsv2rgb(hue, color.s, color.v);
    // can't use hue2pix here
    hue = ++idx * 360.0 / pick_slider_rect.inner().width();
  }
}

void HueSliderWidget::renderBackground(QPainter &) {}

namespace {

int hue2pix(const int hue) {
  return qRound(hue / 359.0 * (pick_slider_rect.inner().width() - 1_px));
}

int pix2hue(const int pix) {
  return std::clamp(qRound(pix * 359.0 / (pick_slider_rect.inner().width() - 1_px)), 0, 359);
}

}

int HueSliderWidget::getPixel() {
  return hue2pix(color.h);
}

void HueSliderWidget::setColor(const int pointX) {
  const int hue = pix2hue(pointX);
  if (hue != color.h) {
    color.h = hue;
    repaint();
    Q_EMIT hueChanged(hue);
  }
}

AlphaSliderWidget::AlphaSliderWidget(QWidget *parent)
  : ColorSliderWidget{parent},
    color{color2hsv(pick_default_color)},
    alpha{pick_default_color.alpha()} {
  plotGraph();
}

void AlphaSliderWidget::changeAlpha(const int newAlpha) {
  alpha = newAlpha;
  repaint();
}

void AlphaSliderWidget::changeHue(const int hue) {
  color.h = hue;
  plotGraph();
  repaint();
}

void AlphaSliderWidget::changeSV(const int sat, const int val) {
  color.s = sat;
  color.v = val;
  plotGraph();
  repaint();
}

void AlphaSliderWidget::changeHSV(const HSV hsv) {
  color = hsv;
  plotGraph();
  repaint();
}

namespace {

QRgb setAlpha(const QRgb color, const int alpha) {
  return qPremultiply(qRgba(qRed(color), qGreen(color), qBlue(color), alpha));
}

}

void AlphaSliderWidget::plotGraph() {
  QRgb *pixels = reinterpret_cast<QRgb *>(graph.bits());
  QRgb *const pixelsEnd = pixels + graph.width();
  const QRgb rgb = hsv2rgb(color.h, color.s, color.v);
  int alp = 0;
  int idx = 0;
  
  while (pixels != pixelsEnd) {
    *pixels++ = setAlpha(rgb, alp);
    // can't use alp2pix here
    alp = qRound(++idx * 255.0 / (pick_slider_rect.inner().width() - 1));
  }
}

void AlphaSliderWidget::renderBackground(QPainter &painter) {
  paintChecker(painter, pick_slider_rect, pick_alpha_tiles);
}

namespace {

int alp2pix(const int alp) {
  return qRound(alp / 255.0 * (pick_slider_rect.inner().width() - 1_px));
}

int pix2alp(const int pix) {
  return std::clamp(qRound(pix * 255.0 / (pick_slider_rect.inner().width() - 1_px)), 0, 255);
}

}

int AlphaSliderWidget::getPixel() {
  return alp2pix(alpha);
}

void AlphaSliderWidget::setColor(const int pointX) {
  const int alp = pix2alp(pointX);
  if (alp != alpha) {
    alpha = alp;
    repaint();
    Q_EMIT alphaChanged(alpha);
  }
}

#include "color slider widget.moc"
