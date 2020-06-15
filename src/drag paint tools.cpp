//
//  drag paint tools.cpp
//  Animera
//
//  Created by Indiana Kernick on 14/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "drag paint tools.hpp"

#include <cmath>
#include "painting.hpp"
#include "scope time.hpp"
#include "config colors.hpp"
#include "graphics convert.hpp"

template <typename Derived>
DragPaintTool<Derived>::~DragPaintTool() {
  static_assert(std::is_base_of_v<DragPaintTool, Derived>);
}

template <typename Derived>
void DragPaintTool<Derived>::attachCel() {}

template <typename Derived>
void DragPaintTool<Derived>::detachCel() {}

template <typename Derived>
void DragPaintTool<Derived>::mouseLeave(const ToolLeaveEvent &event) {
  SCOPE_TIME("DragPaintTool::mouseLeave");

  ctx->clearStatus();
  that()->drawPoint(*ctx->overlay, {}, event.lastPos);
  ctx->changeOverlay(that()->pointRect(event.lastPos));
}

template <typename Derived>
void DragPaintTool<Derived>::mouseDown(const ToolMouseDownEvent &event) {
  SCOPE_TIME("DragPaintTool::mouseDown");

  StatusMsg status;
  that()->updateStatus(status, event.pos, event.pos);
  ctx->showStatus(status);
  
  startPos = event.pos;
  color = ctx->selectColor(event.button);
  const QRect rect = that()->pointRect(event.pos);
  if (!color.zero()) ctx->growCel(rect);
  cleanCel = *ctx->cel;
  const QPoint pos = ctx->cel->pos;
  that()->drawPoint(ctx->cel->img, color, startPos - pos);
  ctx->changeCel(rect);
  ctx->lock();
}

template <typename Derived>
void DragPaintTool<Derived>::mouseMove(const ToolMouseMoveEvent &event) {
  SCOPE_TIME("DragPaintTool::mouseMove");

  that()->drawPoint(*ctx->overlay, {}, event.lastPos);
  that()->drawPoint(*ctx->overlay, PixelVar{tool_overlay_color}, event.pos);
  
  if (event.button == ButtonType::none) {
    ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
    const QRect lastRect = that()->pointRect(event.lastPos);
    ctx->changeOverlay(lastRect.united(that()->pointRect(event.pos)));
    return;
  }
  
  StatusMsg status;
  that()->updateStatus(status, startPos, event.pos);
  ctx->showStatus(status);
  
  *ctx->cel = cleanCel;
  const QRect rect = that()->dragRect(startPos, event.pos);
  if (!color.zero()) ctx->growCel(rect);
  const QPoint pos = ctx->cel->pos;
  that()->drawDrag(ctx->cel->img, startPos - pos, event.pos - pos);
  ctx->changeCel(rect.united(that()->dragRect(startPos, event.lastPos)));
}

template <typename Derived>
void DragPaintTool<Derived>::mouseUp(const ToolMouseUpEvent &event) {
  SCOPE_TIME("DragPaintTool::mouseUp");

  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  ctx->unlock();
  if (color.zero()) ctx->shrinkCel(that()->dragRect(startPos, event.pos));
  ctx->finishChange();
}

template <typename Derived>
PixelVar DragPaintTool<Derived>::getColor() const {
  return color;
}

template <typename Derived>
Derived *DragPaintTool<Derived>::that() {
  return static_cast<Derived *>(this);
}

LineTool::~LineTool() = default;

void LineTool::setRadius(const int newRadius) {
  assert(line_radius.min <= newRadius && newRadius <= line_radius.max);
  radius = newRadius;
}

bool LineTool::drawPoint(QImage &image, const PixelVar col, const QPoint pos) {
  return drawRoundPoint(image, col, pos, radius);
}

bool LineTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawLine(image, getColor(), {start, end}, radius);
}

void LineTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("Start: ");
  status.append(start);
  status.append(" End: ");
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
  assert(circ_thick.min <= newThick && newThick <= circ_thick.max);
  thickness = newThick;
}

bool StrokedCircleTool::drawPoint(QImage &image, const PixelVar col, const QPoint pos) {
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
  status.append("Center: ");
  status.append(start);
  status.append(" Radius: ");
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

bool FilledCircleTool::drawPoint(QImage &image, const PixelVar col, const QPoint pos) {
  return drawSquarePoint(image, col, pos, shape);
}

bool FilledCircleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawFilledCircle(image, getColor(), start, calcRadius(start, end), shape);
}

void FilledCircleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("Center: ");
  status.append(start);
  status.append(" Radius: ");
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
  assert(rect_thick.min <= newThick && newThick <= rect_thick.max);
  thickness = newThick;
}

bool StrokedRectangleTool::drawPoint(QImage &image, const PixelVar col, const QPoint pos) {
  return drawSquarePoint(image, col, pos);
}

bool StrokedRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawStrokedRect(image, getColor(), unite(start, end), thickness);
}

void StrokedRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("Rect: ");
  status.append(unite(start, end));
}

QRect StrokedRectangleTool::pointRect(const QPoint pos) {
  return toRect(pos);
}

QRect StrokedRectangleTool::dragRect(const QPoint start, const QPoint end) {
  return unite(start, end);
}

FilledRectangleTool::~FilledRectangleTool() = default;

bool FilledRectangleTool::drawPoint(QImage &image, const PixelVar col, const QPoint pos) {
  return drawSquarePoint(image, col, pos);
}

bool FilledRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawFilledRect(image, getColor(), unite(start, end));
}

void FilledRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("Rect: ");
  status.append(unite(start, end));
}

QRect FilledRectangleTool::pointRect(const QPoint pos) {
  return toRect(pos);
}

QRect FilledRectangleTool::dragRect(const QPoint start, const QPoint end) {
  return unite(start, end);
}
