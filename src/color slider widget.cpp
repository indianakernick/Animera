//
//  color slider widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "color slider widget.hpp"

#include "config.hpp"
#include "status msg.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include "widget painting.hpp"
#include "surface factory.hpp"
#include <Graphics/iterator.hpp>

namespace {

constexpr int slider_width = pick_slider_rect.inner().width();

}

template <typename Derived>
ColorSliderWidget<Derived>::ColorSliderWidget(QWidget *parent)
  : QWidget{parent},
    graph{slider_width, 1, QImage::Format_ARGB32_Premultiplied} {
  setMouseTracking(true);
  setFocusPolicy(Qt::ClickFocus);
  setFixedSize(pick_slider_rect.widget().size());
  initBar();
  show();
}

template <typename Derived>
void ColorSliderWidget<Derived>::initBar() {
  bar = bakeColoredBitmaps(
    ":/Color Picker/slider bar p.png",
    ":/Color Picker/slider bar s.png",
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
  const int pointX = event->localPos().x() - pick_slider_rect.inner().x();
  static_cast<Derived *>(this)->setColor(pointX);
}

template <typename Derived>
void ColorSliderWidget<Derived>::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mouseDown = true;
    setColor(event);
  }
}

template <typename Derived>
void ColorSliderWidget<Derived>::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mouseDown = false;
    setColor(event);
  }
}

template <typename Derived>
void ColorSliderWidget<Derived>::mouseMoveEvent(QMouseEvent *event) {
  if (mouseDown) {
    setColor(event);
  }
  StatusMsg status;
  const int pointX = event->localPos().x() - pick_slider_rect.inner().x();
  static_cast<Derived *>(this)->updateStatus(status, pointX);
  Q_EMIT static_cast<Derived *>(this)->shouldShowNorm(status.get());
}

template <typename Derived>
void ColorSliderWidget<Derived>::leaveEvent(QEvent *) {
  Q_EMIT static_cast<Derived *>(this)->shouldShowNorm("");
}

template <typename Derived>
void ColorSliderWidget<Derived>::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Right) {
    static_cast<Derived *>(this)->incColor();
  } else if (event->key() == Qt::Key_Left) {
    static_cast<Derived *>(this)->decColor();
  }
}

HueSliderWidget::HueSliderWidget(QWidget *parent, const HSV color, const int alpha)
  : ColorSliderWidget{parent},
    color{color},
    alpha{alpha} {
  plotGraph();
}

void HueSliderWidget::setHue(const int hue) {
  color.h = hue;
  repaint();
}

void HueSliderWidget::setSV(const int sat, const int val) {
  color.s = sat;
  color.v = val;
  plotGraph();
  repaint();
}

void HueSliderWidget::setHSV(const HSV hsv) {
  color = hsv;
  plotGraph();
  repaint();
}

void HueSliderWidget::setAlpha(const int newAlpha) {
  alpha = newAlpha;
}

void HueSliderWidget::plotGraph() {
  int idx = 0;
  for (QRgb &pixel : *gfx::begin(makeSurface<QRgb>(graph))) {
    // can't use pix2hue here
    const qreal hue = idx++ * 360.0 / slider_width;
    pixel = hsv2rgb(hue, color.s, color.v);
  }
}

void HueSliderWidget::renderBackground(QPainter &) {}

namespace {

int hue2pix(const int hue) {
  return scale(hue, 359, slider_width - 1_px);
}

int pix2hue(const int pix) {
  return std::clamp(scale(pix, slider_width - 1_px, 359), 0, 359);
}

}

void HueSliderWidget::updateHue(const int hue) {
  if (hue != color.h) {
    color.h = hue;
    repaint();
    Q_EMIT hueChanged(hue);
  }
}

int HueSliderWidget::getPixel() {
  return hue2pix(color.h);
}

void HueSliderWidget::setColor(const int pointX) {
  updateHue(pix2hue(pointX));
}

void HueSliderWidget::incColor() {
  updateHue(std::min(color.h + 1, 359));
}

void HueSliderWidget::decColor() {
  updateHue(std::max(color.h - 1, 0));
}

void HueSliderWidget::updateStatus(StatusMsg &status, const int pointX) {
  const int hue = pix2hue(pointX);
  status.append("HUE: ");
  status.append(hue);
  status.append(" COLOR: ");
  const RGB rgbColor = hsv2rgb({hue, color.s, color.v});
  status.append(rgbColor.r, rgbColor.g, rgbColor.b, alpha);
}

AlphaSliderWidget::AlphaSliderWidget(
  QWidget *parent,
  const RGB color,
  const int alpha,
  const bool grayMode
) : ColorSliderWidget{parent},
    color{color},
    alpha{alpha},
    grayMode{grayMode} {
  plotGraph();
}

void AlphaSliderWidget::setAlpha(const int newAlpha) {
  alpha = newAlpha;
  repaint();
}

void AlphaSliderWidget::setRgba(const RGB rgb, const int alp) {
  color = rgb;
  alpha = alp;
  plotGraph();
  repaint();
}

void AlphaSliderWidget::plotGraph() {
  int idx = 0;
  for (QRgb &pixel : *gfx::begin(makeSurface<QRgb>(graph))) {
    // can't use pix2alp here
    const int alp = scale(idx++, slider_width - 1, 255);
    pixel = qPremultiply(qRgba(color.r, color.g, color.b, alp));
  }
}

void AlphaSliderWidget::renderBackground(QPainter &painter) {
  // TODO: consider baking the checkerboard
  paintChecker(painter, pick_slider_rect, pick_alpha_tiles);
}

namespace {

int alp2pix(const int alp) {
  return scale(alp, 255, slider_width - 1_px);
}

int pix2alp(const int pix) {
  return std::clamp(scale(pix, slider_width - 1_px, 255), 0, 255);
}

}

void AlphaSliderWidget::updateAlpha(const int alp) {
  if (alp != alpha) {
    alpha = alp;
    repaint();
    Q_EMIT alphaChanged(alpha);
  }
}

int AlphaSliderWidget::getPixel() {
  return alp2pix(alpha);
}

void AlphaSliderWidget::setColor(const int pointX) {
  updateAlpha(pix2alp(pointX));
}

void AlphaSliderWidget::incColor() {
  updateAlpha(std::min(alpha + 1, 255));
}

void AlphaSliderWidget::decColor() {
  updateAlpha(std::max(alpha - 1, 0));
}

void AlphaSliderWidget::updateStatus(StatusMsg &status, const int pointX) {
  const int alphaValue = pix2alp(pointX);
  status.append("ALPHA: ");
  status.append(alphaValue);
  status.append(" COLOR: ");
  if (grayMode) {
    status.append(color.r, alphaValue);
  } else {
    status.append(color.r, color.g, color.b, alphaValue);
  }
}

GraySliderWidget::GraySliderWidget(QWidget *parent, const int gray, const int alpha)
  : ColorSliderWidget{parent},
    gray{gray},
    alpha{alpha} {
  plotGraph();
}

void GraySliderWidget::setGray(const int newGray) {
  gray = newGray;
  repaint();
}

void GraySliderWidget::setAlpha(const int newAlpha) {
  alpha = newAlpha;
}

void GraySliderWidget::plotGraph() {
  int idx = 0;
  for (QRgb &pixel : *gfx::begin(makeSurface<QRgb>(graph))) {
    // can't use pix2alp here
    const int y = scale(idx++, slider_width - 1, 255);
    pixel = qRgb(y, y, y);
  }
}

void GraySliderWidget::renderBackground(QPainter &) {}

void GraySliderWidget::updateGray(const int newGray) {
  if (newGray != gray) {
    gray = newGray;
    repaint();
    Q_EMIT grayChanged(gray);
  }
}

int GraySliderWidget::getPixel() {
  return alp2pix(gray);
}

void GraySliderWidget::setColor(const int pointX) {
  updateGray(pix2alp(pointX));
}

void GraySliderWidget::incColor() {
  updateGray(std::min(gray + 1, 255));
}

void GraySliderWidget::decColor() {
  updateGray(std::max(gray - 1, 0));
}

void GraySliderWidget::updateStatus(StatusMsg &status, const int pointX) {
  const int grayValue = pix2alp(pointX);
  status.append("GRAY: ");
  status.append(grayValue);
  status.append(" COLOR: ");
  status.append(grayValue, alpha);
}

#include "color slider widget.moc"
