//
//  paint tool impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "paint tool impls.hpp"

#include <cmath>
#include "painting.hpp"
#include "cell impls.hpp"
#include <QtGui/qpainter.h>

namespace {

void initPainter(QPainter &painter, SourceCell *source) {
  assert(source);
  painter.begin(&source->image.data);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.setTransform(getInvTransform(source->image));
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
  // the QRgb constructor sets alpha to 255 for some reason
  return QColor{qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba)};
}

const QColor overlay_color{
  overlay_gray, overlay_gray, overlay_gray, overlay_alpha
};
const QPen round_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin
};
const QPen square_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin
};

void clearOverlay(QImage *overlay) {
  assert(overlay);
  clearImage(*overlay);
}

void drawPointOverlay(QImage *overlay, const QPoint pos, QPen colorPen) {
  assert(overlay);
  QPainter painter{overlay};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setRenderHint(QPainter::Antialiasing, false);
  colorPen.setColor(overlay_color);
  painter.setPen(colorPen);
  painter.drawPoint(pos);
}

}

BrushTool::BrushTool()
  : pen{round_pen} {}

bool BrushTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

ToolChanges BrushTool::mouseDown(const ToolEvent &event) {
  assert(source);
  if (button != ButtonType::none) return ToolChanges::none;
  clearOverlay(event.overlay);
  drawPointOverlay(event.overlay, event.pos, pen);
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
  drawPointOverlay(event.overlay, event.pos, pen);
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
  assert(min_thickness <= diameter && diameter <= max_thickness);
  pen.setWidth(diameter);
}

int BrushTool::getDiameter() const {
  return pen.width();
}

bool FloodFillTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

ToolChanges FloodFillTool::mouseDown(const ToolEvent &event) {
  assert(source);
  lastPos = event.pos;
  clearOverlay(event.overlay);
  drawPointOverlay(event.overlay, event.pos, square_pen);
  const QRgb toolColor = selectColor(event.colors, event.type);
  const QTransform xform = getInvTransform(source->image);
  const bool drawn = drawFloodFill(source->image.data, toolColor, xform.map(event.pos));
  return drawn ? ToolChanges::cell_overlay : ToolChanges::overlay;
}

ToolChanges FloodFillTool::mouseMove(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  lastPos = event.pos;
  clearOverlay(event.overlay);
  drawPointOverlay(event.overlay, event.pos, square_pen);
  return ToolChanges::overlay;
}

ToolChanges FloodFillTool::mouseUp(const ToolEvent &) {
  assert(source);
  return ToolChanges::none;
}

template <typename Derived>
DragPaintTool<Derived>::~DragPaintTool() {
  static_assert(std::is_base_of_v<DragPaintTool, Derived>);
}

template <typename Derived>
bool DragPaintTool<Derived>::attachCell(Cell *cell) {
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

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseDown(const ToolEvent &event) {
  assert(source);
  if (button != ButtonType::none) return ToolChanges::none;
  clearOverlay(event.overlay);
  that()->drawOverlay(event.overlay, event.pos);
  button = event.type;
  startPos = lastPos = event.pos;
  copyImage(cleanImage, source->image.data);
  that()->setColor(toColor(selectColor(event.colors, event.type)));
  QPainter painter;
  initPainter(painter, source);
  that()->setupPainter(painter);
  that()->drawPoint(painter, startPos);
  return ToolChanges::cell_overlay;
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseMove(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  clearOverlay(event.overlay);
  that()->drawOverlay(event.overlay, event.pos);
  if (event.type == ButtonType::none) return ToolChanges::overlay;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  QPainter painter;
  initPainter(painter, source);
  that()->setupPainter(painter);
  that()->drawDrag(painter, startPos, lastPos);
  return ToolChanges::cell_overlay;
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseUp(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  if (event.type != button) return ToolChanges::none;
  clearOverlay(event.overlay);
  button = ButtonType::none;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  QPainter painter;
  initPainter(painter, source);
  that()->setupPainter(painter);
  that()->drawDrag(painter, startPos, lastPos);
  startPos = no_point;
  return ToolChanges::cell_overlay;
}

template <typename Derived>
bool DragPaintTool<Derived>::isDragging() const {
  return startPos != no_point;
}

template <typename Derived>
Derived *DragPaintTool<Derived>::that() {
  return static_cast<Derived *>(this);
}

LineTool::LineTool()
  : pen{round_pen} {}

LineTool::~LineTool() = default;

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
  drawPointOverlay(overlay, pos, pen);
}

StrokedCircleTool::StrokedCircleTool()
  : pen{round_pen} {}

StrokedCircleTool::~StrokedCircleTool() = default;

void StrokedCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
}

CircleShape StrokedCircleTool::getShape() const {
  return shape;
}

int StrokedCircleTool::getRadius() const {
  return isDragging() ? radius : no_radius;
}

void StrokedCircleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void StrokedCircleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
}

void StrokedCircleTool::drawPoint(QPainter &, QPoint) {
  radius = 0;
}

namespace {

double distance(const QPoint a, const QPoint b) {
  const int dx = a.x() - b.x();
  const int dy = a.y() - b.y();
  return std::sqrt(dx*dx + dy*dy);
}

int calcRadius(const QPoint start, const QPoint end) {
  return std::round(distance(start, end));
}

}

void StrokedCircleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  radius = calcRadius(start, end);
  painter.drawEllipse(adjustStrokedEllipse(circleToRect(start, radius, shape), 1));
}

void StrokedCircleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}

FilledCircleTool::FilledCircleTool()
  : pen{round_pen} {}

FilledCircleTool::~FilledCircleTool() = default;

void FilledCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
}

CircleShape FilledCircleTool::getShape() const {
  return shape;
}

int FilledCircleTool::getRadius() const {
  return isDragging() ? radius : no_radius;
}

void FilledCircleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void FilledCircleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
  painter.setBrush(pen.color());
}

void FilledCircleTool::drawPoint(QPainter &, QPoint) {
  radius = 0;
}

void FilledCircleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  radius = calcRadius(start, end);
  painter.drawEllipse(adjustStrokedEllipse(circleToRect(start, radius, shape), 1));
}

void FilledCircleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}

StrokedRectangleTool::StrokedRectangleTool()
  : pen{square_pen} {}

StrokedRectangleTool::~StrokedRectangleTool() = default;

QSize StrokedRectangleTool::getSize() const {
  return isDragging() ? size : no_size;
}

void StrokedRectangleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void StrokedRectangleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
}

void StrokedRectangleTool::drawPoint(QPainter &painter, const QPoint pos) {
  size = QSize{1, 1};
  painter.drawPoint(pos);
}

namespace {

QRect calcRect(const QPoint start, const QPoint end) {
  return QRect{start, end}.normalized();
}

}

void StrokedRectangleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  if (start == end) {
    drawPoint(painter, start);
  } else {
    const QRect rect = calcRect(start, end);
    size = rect.size();
    painter.drawRect(adjustStrokedRect(rect, pen.width()));
  }
}

void StrokedRectangleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}

FilledRectangleTool::FilledRectangleTool()
  : pen{square_pen} {}

FilledRectangleTool::~FilledRectangleTool() = default;

QSize FilledRectangleTool::getSize() const {
  return isDragging() ? size : no_size;
}

void FilledRectangleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void FilledRectangleTool::setupPainter(QPainter &painter) {
  painter.setPen(Qt::NoPen);
  painter.setBrush(pen.color());
}

void FilledRectangleTool::drawPoint(QPainter &painter, const QPoint pos) {
  size = QSize{1, 1};
  painter.drawPoint(pos);
}

void FilledRectangleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  const QRect rect = calcRect(start, end);
  size = rect.size();
  painter.drawRect(rect);
}

void FilledRectangleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}
