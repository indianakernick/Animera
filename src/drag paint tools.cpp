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
void DragPaintTool<Derived>::mouseLeave(const ToolLeaveEvent &event) {
  ctx->clearStatus();
  that()->drawPoint(*ctx->overlay, 0, event.lastPos);
  ctx->changeOverlay(that()->pointRect(event.lastPos));
}

template <typename Derived>
void DragPaintTool<Derived>::mouseDown(const ToolMouseEvent &event) {
  StatusMsg status = ctx->showStatus();
  that()->updateStatus(status, event.pos, event.pos);
  
  startPos = event.pos;
  color = ctx->selectColor(event.button);
  const QRect rect = that()->pointRect(event.pos);
  ctx->growCell(rect);
  cleanCell = *ctx->cell;
  const QPoint pos = ctx->cell->pos;
  that()->drawPoint(ctx->cell->img, color, startPos - pos);
  ctx->changeCell(rect);
  ctx->lock();
}

template <typename Derived>
void DragPaintTool<Derived>::mouseMove(const ToolMouseEvent &event) {
  that()->drawPoint(*ctx->overlay, 0, event.lastPos);
  that()->drawPoint(*ctx->overlay, tool_overlay_color, event.pos);
  
  if (event.button == ButtonType::none) {
    ctx->showStatus().appendLabeled(event.pos);
    ctx->changeOverlay(that()->pointRect(event.lastPos));
    ctx->changeOverlay(that()->pointRect(event.pos));
    return;
  }
  
  StatusMsg status = ctx->showStatus();
  that()->updateStatus(status, startPos, event.pos);
  
  *ctx->cell = cleanCell;
  const QRect rect = that()->dragRect(startPos, event.pos);
  ctx->growCell(rect);
  const QPoint pos = ctx->cell->pos;
  that()->drawDrag(ctx->cell->img, startPos - pos, event.pos - pos);
  ctx->changeCell(rect.united(that()->dragRect(startPos, event.lastPos)));
}

template <typename Derived>
void DragPaintTool<Derived>::mouseUp(const ToolMouseEvent &event) {
  ctx->showStatus().appendLabeled(event.pos);
  ctx->unlock();
  if (color == 0) ctx->shrinkCell();
  ctx->finishChange();
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

bool LineTool::drawPoint(QImage &image, const QRgb col, const QPoint pos) {
  return drawRoundPoint(image, col, pos, radius);
}

bool LineTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawLine(image, getColor(), {start, end}, radius);
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

QRect LineTool::dragRect(const QPoint start, const QPoint end) {
  return pointRect(start).united(pointRect(end));
}

StrokedCircleTool::~StrokedCircleTool() = default;

void StrokedCircleTool::setShape(const gfx::CircleShape newShape) {
  shape = newShape;
}

void StrokedCircleTool::setThick(const int newThick) {
  thickness = newThick;
}

bool StrokedCircleTool::drawPoint(QImage &image, const QRgb col, const QPoint pos) {
  return drawSquarePoint(image, col, pos, shape);
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

bool FilledCircleTool::drawPoint(QImage &image, const QRgb col, const QPoint pos) {
  return drawSquarePoint(image, col, pos, shape);
}

bool FilledCircleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawFilledCircle(image, getColor(), start, calcRadius(start, end), shape);
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

bool StrokedRectangleTool::drawPoint(QImage &image, const QRgb col, const QPoint pos) {
  return drawSquarePoint(image, col, pos);
}

bool StrokedRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawStrokedRect(image, getColor(), unite(start, end), thickness);
}

void StrokedRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("RECT: ");
  status.append(unite(start, end));
}

QRect StrokedRectangleTool::pointRect(const QPoint pos) {
  return toRect(pos);
}

QRect StrokedRectangleTool::dragRect(const QPoint start, const QPoint end) {
  return unite(start, end);
}

FilledRectangleTool::~FilledRectangleTool() = default;

bool FilledRectangleTool::drawPoint(QImage &image, const QRgb col, const QPoint pos) {
  return drawSquarePoint(image, col, pos);
}

bool FilledRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawFilledRect(image, getColor(), unite(start, end));
}

void FilledRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("RECT: ");
  status.append(unite(start, end));
}

QRect FilledRectangleTool::pointRect(const QPoint pos) {
  return toRect(pos);
}

QRect FilledRectangleTool::dragRect(const QPoint start, const QPoint end) {
  return unite(start, end);
}
