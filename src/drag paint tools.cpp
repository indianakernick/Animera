//
//  drag paint tools.cpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "drag paint tools.hpp"

#include <cmath>
#include "painting.hpp"
#include "graphics convert.hpp"

template <typename Derived>
DragPaintTool<Derived>::~DragPaintTool() {
  static_assert(std::is_base_of_v<DragPaintTool, Derived>);
}

template <typename Derived>
void DragPaintTool<Derived>::attachCell() {}

template <typename Derived>
void DragPaintTool<Derived>::detachCell() {}

template <typename Derived>
void DragPaintTool<Derived>::mouseLeave(const ToolLeaveEvent &) {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

template <typename Derived>
void DragPaintTool<Derived>::mouseDown(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  that()->drawOverlay(*ctx->overlay, event.pos);
  ctx->growCell(that()->pointRect(event.pos));
  StatusMsg status;
  that()->updateStatus(status, event.pos, event.pos);
  ctx->showStatus(status);
  startPos = event.pos;
  color = ctx->selectColor(event.button);
  cleanCell = *ctx->cell;
  QImage &img = ctx->cell->img;
  const QPoint pos = ctx->cell->pos;
  ctx->emitChanges(that()->drawPoint(img, startPos - pos));
  ctx->lock();
}

template <typename Derived>
void DragPaintTool<Derived>::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  that()->drawOverlay(*ctx->overlay, event.pos);
  if (event.button == ButtonType::none) {
    ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
    return ctx->emitChanges(ToolChanges::overlay);
  }
  *ctx->cell = cleanCell;
  ctx->growCell(that()->dragRect(startPos, event.pos));
  StatusMsg status;
  that()->updateStatus(status, startPos, event.pos);
  ctx->showStatus(status);
  QImage &img = ctx->cell->img;
  const QPoint pos = ctx->cell->pos;
  ctx->emitChanges(that()->drawDrag(img, startPos - pos, event.pos - pos));
}

template <typename Derived>
void DragPaintTool<Derived>::mouseUp(const ToolMouseEvent &event) {
  // should probably set overlay to mouse position
  ctx->unlock();
  clearImage(*ctx->overlay);
  that()->drawOverlay(*ctx->overlay, event.pos);
  *ctx->cell = cleanCell;
  ctx->growCell(that()->dragRect(startPos, event.pos));
  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  QImage &img = ctx->cell->img;
  const QPoint pos = ctx->cell->pos;
  ctx->emitChanges(that()->drawDrag(img, startPos - pos, event.pos - pos));
  ctx->finishChange();
  startPos = no_point;
}

template <typename Derived>
bool DragPaintTool<Derived>::isDragging() const {
  return startPos != no_point;
}

template <typename Derived>
QRgb DragPaintTool<Derived>::getColor() const {
  return color;
}

template <typename Derived>
Derived *DragPaintTool<Derived>::that() {
  return static_cast<Derived *>(this);
}

LineTool::~LineTool() = default;

void LineTool::setRadius(const int newRadius) {
  radius = newRadius;
}

bool LineTool::drawPoint(QImage &image, const QPoint pos) {
  return drawRoundPoint(image, getColor(), pos, radius);
}

bool LineTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawLine(image, getColor(), {start, end}, radius);
}

void LineTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawRoundPoint(overlay, tool_overlay_color, pos, radius);
}

void LineTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("START: ");
  status.append(start);
  status.append(" END: ");
  status.append(end);
}

QRect LineTool::pointRect(const QPoint pos) {
  return convert(gfx::circleRect(convert(pos), radius));
}

QRect LineTool::dragRect(QPoint, const QPoint end) {
  return pointRect(end);
}

StrokedCircleTool::~StrokedCircleTool() = default;

void StrokedCircleTool::setShape(const gfx::CircleShape newShape) {
  shape = newShape;
}

void StrokedCircleTool::setThick(const int newThick) {
  thickness = newThick;
}

bool StrokedCircleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos, shape);
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

bool StrokedCircleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawStrokedCircle(image, getColor(), start, calcRadius(start, end), thickness, shape);
}

void StrokedCircleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawFilledRect(overlay, tool_overlay_color, convert(gfx::centerRect(convert(pos), shape)));
}

void StrokedCircleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("CENTER: ");
  status.append(start);
  status.append(" RADIUS: ");
  status.append(calcRadius(start, end));
}

QRect StrokedCircleTool::pointRect(const QPoint pos) {
  return convert(gfx::centerRect(convert(pos), shape));
}

QRect StrokedCircleTool::dragRect(const QPoint start, const QPoint end) {
  return convert(gfx::circleRect(convert(start), calcRadius(start, end), shape));
}

FilledCircleTool::~FilledCircleTool() = default;

void FilledCircleTool::setShape(const gfx::CircleShape newShape) {
  shape = newShape;
}

bool FilledCircleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos, shape);
}

bool FilledCircleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawFilledCircle(image, getColor(), start, calcRadius(start, end), shape);
}

void FilledCircleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawFilledRect(overlay, tool_overlay_color, convert(gfx::centerRect(convert(pos), shape)));
}

void FilledCircleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("CENTER: ");
  status.append(start);
  status.append(" RADIUS: ");
  status.append(calcRadius(start, end));
}

QRect FilledCircleTool::pointRect(const QPoint pos) {
  return convert(gfx::centerRect(convert(pos), shape));
}

QRect FilledCircleTool::dragRect(const QPoint start, const QPoint end) {
  return convert(gfx::circleRect(convert(start), calcRadius(start, end), shape));
}

StrokedRectangleTool::~StrokedRectangleTool() = default;

void StrokedRectangleTool::setThick(const int newThick) {
  thickness = newThick;
}

bool StrokedRectangleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos);
}

bool StrokedRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawStrokedRect(image, getColor(), unite(start, end), thickness);
}

void StrokedRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, tool_overlay_color, pos);
}

void StrokedRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.appendLabeled(unite(start, end));
}

QRect StrokedRectangleTool::pointRect(const QPoint pos) {
  return toRect(pos);
}

QRect StrokedRectangleTool::dragRect(const QPoint start, const QPoint end) {
  return unite(start, end);
}

FilledRectangleTool::~FilledRectangleTool() = default;

bool FilledRectangleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos);
}

bool FilledRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  const QRect rect = unite(start, end);
  return drawFilledRect(image, getColor(), rect);
}

void FilledRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, tool_overlay_color, pos);
}

void FilledRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.appendLabeled(unite(start, end));
}

QRect FilledRectangleTool::pointRect(const QPoint pos) {
  return toRect(pos);
}

QRect FilledRectangleTool::dragRect(const QPoint start, const QPoint end) {
  return unite(start, end);
}
