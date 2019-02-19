//
//  paint tool impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "paint tool impls.hpp"

#include <cmath>
#include "cell impls.hpp"
#include <QtGui/qpainter.h>

namespace {

void initPainter(QPainter &painter, SourceCell *source) {
  assert(source);
  painter.begin(&source->image.data);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.resetTransform();
  applyInvTransform(painter, source->image);
}

QRgb selectColor(const ToolColors &colors, const ButtonType button) {
  switch (button) {
    case ButtonType::primary: return colors.primary;
    case ButtonType::secondary: return colors.secondary;
    case ButtonType::erase: return colors.erase;
    default: Q_UNREACHABLE();
  }
  return 0;
}

QColor toColor(const QRgb rgba) {
  // the QRgb constructor just sets alpha to 255 for some reason
  return QColor{qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba)};
}

const QColor overlay_color{
  overlay_gray, overlay_gray, overlay_gray, overlay_alpha
};
const QPen default_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin
};

void clearOverlay(QImage *overlay) {
  assert(overlay);
  overlay->fill(0);
}

void drawOverlay(QImage *overlay, const QPoint pos, QPen colorPen) {
  assert(overlay);
  QPainter painter{overlay};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setRenderHint(QPainter::Antialiasing, false);
  colorPen.setColor(overlay_color);
  painter.setPen(colorPen);
  painter.drawPoint(pos);
}

bool compatible(const QImage &a, const QImage &b) {
  return a.size() == b.size() && a.format() == b.format();
}

QImage makeCompatible(const QImage &img) {
  return QImage{img.size(), img.format()};
}

void copyImage(QImage &dst, const QImage &src) {
  assert(compatible(dst, src));
  dst.detach();
  std::memcpy(dst.bits(), src.constBits(), dst.sizeInBytes());
}

}

BrushTool::BrushTool()
  : pen{default_pen} {}

bool BrushTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

ToolChanges BrushTool::mouseDown(const ToolEvent &event) {
  assert(source);
  if (button != ButtonType::none) return ToolChanges::none;
  clearOverlay(event.overlay);
  drawOverlay(event.overlay, event.pos, pen);
  button = event.type;
  lastPos = event.pos;
  pen.setColor(toColor(selectColor(event.colors, event.type)));
  QPainter painter;
  initPainter(painter, source);
  painter.setPen(pen);
  painter.drawPoint(lastPos);
  return ToolChanges::cell_overlay;
}

ToolChanges BrushTool::mouseMove(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  clearOverlay(event.overlay);
  drawOverlay(event.overlay, event.pos, pen);
  if (event.type == ButtonType::none) return ToolChanges::overlay;
  QPainter painter;
  initPainter(painter, source);
  painter.setPen(pen);
  painter.drawLine(lastPos, event.pos);
  lastPos = event.pos;
  return ToolChanges::cell_overlay;
}

ToolChanges BrushTool::mouseUp(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  if (event.type != button) return ToolChanges::none;
  clearOverlay(event.overlay);
  button = ButtonType::none;
  QPainter painter;
  initPainter(painter, source);
  painter.setPen(pen);
  painter.drawLine(lastPos, event.pos);
  return ToolChanges::cell_overlay;
}

void BrushTool::setDiameter(const int diameter) {
  assert(min_diameter <= diameter && diameter <= max_diameter);
  pen.setWidth(diameter);
}

int BrushTool::getDiameter() const {
  return pen.width();
}

bool DragPaintTool::attachCell(Cell *cell) {
  source = dynamic_cast<SourceCell *>(cell);
  if (source) {
    if (!compatible(cleanImage, source->image.data)) {
      cleanImage = makeCompatible(source->image.data);
    }
    return true;
  } else {
    return false;
  }
}

ToolChanges DragPaintTool::mouseDown(const ToolEvent &event) {
  assert(source);
  if (button != ButtonType::none) return ToolChanges::none;
  clearOverlay(event.overlay);
  drawOverlay(event.overlay, event.pos);
  button = event.type;
  startPos = lastPos = event.pos;
  copyImage(cleanImage, source->image.data);
  setColor(toColor(selectColor(event.colors, event.type)));
  QPainter painter;
  initPainter(painter, source);
  setupPainter(painter);
  drawPoint(painter, startPos);
  return ToolChanges::cell_overlay;
}

ToolChanges DragPaintTool::mouseMove(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  clearOverlay(event.overlay);
  drawOverlay(event.overlay, event.pos);
  if (event.type == ButtonType::none) return ToolChanges::overlay;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  QPainter painter;
  initPainter(painter, source);
  setupPainter(painter);
  drawDrag(painter, startPos, lastPos);
  return ToolChanges::cell_overlay;
}

ToolChanges DragPaintTool::mouseUp(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  if (event.type != button) return ToolChanges::none;
  clearOverlay(event.overlay);
  button = ButtonType::none;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  QPainter painter;
  initPainter(painter, source);
  setupPainter(painter);
  drawDrag(painter, startPos, lastPos);
  return ToolChanges::cell_overlay;
}

LineTool::LineTool()
  : pen{default_pen} {}

void LineTool::setThickness(const int thickness) {
  assert(min_thickness <= thickness && thickness <= max_thickness);
  pen.setWidth(thickness);
}

int LineTool::getThickness() const {
  return pen.width();
}

void LineTool::setColor(const QColor color) {
  pen.setColor(color);
}

void LineTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
}

void LineTool::drawPoint(QPainter &painter, const QPoint pos) {
  painter.drawPoint(pos);
}

void LineTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  painter.drawLine(start, end);
}

void LineTool::drawOverlay(QImage *overlay, const QPoint pos) {
  ::drawOverlay(overlay, pos, pen);
}

StrokedCircleTool::StrokedCircleTool()
  : pen{default_pen} {}

void StrokedCircleTool::setThickness(const int thickness) {
  assert(min_thickness <= thickness && thickness <= max_thickness);
  pen.setWidth(thickness);
}

int StrokedCircleTool::getThickness() const {
  return pen.width();
}

void StrokedCircleTool::setCenter(const CircleCenter cent) {
  center = cent;
}

CircleCenter StrokedCircleTool::getCenter() const {
  return center;
}

void StrokedCircleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void StrokedCircleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
}

void StrokedCircleTool::drawPoint(QPainter &painter, const QPoint pos) {
  painter.drawPoint(pos);
}

namespace {

int addEllipseWidth(const CircleCenter center) {
  return (center == CircleCenter::c2x1 || center == CircleCenter::c2x2);
}

int addEllipseHeight(const CircleCenter center) {
  return (center == CircleCenter::c1x2 || center == CircleCenter::c2x2);
}

double distance(const QPoint a, const QPoint b) {
  const int dx = a.x() - b.x();
  const int dy = a.y() - b.y();
  return std::sqrt(dx*dx + dy*dy);
}

}

void StrokedCircleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  // @TODO revisit this
  const int radius = distance(start, end);
  const QPoint radiusPoint{radius, radius};
  QRect rect{start - radiusPoint, QSize{radius * 2, radius * 2}};
  rect.setRight(rect.right() + addEllipseWidth(center));
  rect.setBottom(rect.bottom() + addEllipseHeight(center));
  painter.drawEllipse(rect);
}

void StrokedCircleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  ::drawOverlay(overlay, pos, pen);
}
