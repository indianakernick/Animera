//
//  drag paint tools.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "drag paint tools.hpp"

#include <cmath>
#include "cell.hpp"
#include "painting.hpp"

template <typename Derived>
DragPaintTool<Derived>::~DragPaintTool() {
  static_assert(std::is_base_of_v<DragPaintTool, Derived>);
}

template <typename Derived>
void DragPaintTool<Derived>::attachCell() {
  if (!compatible(cleanImage, ctx->cell->image)) {
    cleanImage = makeCompatible(ctx->cell->image);
  }
}

template <typename Derived>
void DragPaintTool<Derived>::detachCell() {
  ctx->clearStatus();
}

template <typename Derived>
void DragPaintTool<Derived>::mouseLeave(const ToolLeaveEvent &) {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

template <typename Derived>
void DragPaintTool<Derived>::mouseDown(const ToolMouseEvent &event) {
  ctx->requireCell();
  clearImage(*ctx->overlay);
  that()->drawOverlay(*ctx->overlay, event.pos);
  StatusMsg status;
  that()->updateStatus(status, event.pos, event.pos);
  ctx->showStatus(status);
  startPos = event.pos;
  copyImage(cleanImage, ctx->cell->image);
  color = ctx->selectColor(event.button);
  ctx->emitChanges(that()->drawPoint(ctx->cell->image, startPos));
}

template <typename Derived>
void DragPaintTool<Derived>::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  that()->drawOverlay(*ctx->overlay, event.pos);
  if (event.button == ButtonType::none) {
    ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
    return ctx->emitChanges(ToolChanges::overlay);
  }
  StatusMsg status;
  that()->updateStatus(status, startPos, event.pos);
  ctx->showStatus(status);
  copyImage(ctx->cell->image, cleanImage);
  ctx->emitChanges(that()->drawDrag(ctx->cell->image, startPos, event.pos));
}

template <typename Derived>
void DragPaintTool<Derived>::mouseUp(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  that()->drawOverlay(*ctx->overlay, event.pos);
  copyImage(ctx->cell->image, cleanImage);
  ctx->emitChanges(that()->drawDrag(ctx->cell->image, startPos, event.pos));
  startPos = no_point;
  ctx->finishChange();
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

StrokedCircleTool::~StrokedCircleTool() = default;

void StrokedCircleTool::setShape(const CircleShape newShape) {
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
  drawFilledRect(overlay, tool_overlay_color, centerRect(pos, shape));
}

void StrokedCircleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("CENTER: ");
  status.append(start);
  status.append(" RADIUS: ");
  status.append(calcRadius(start, end));
}

FilledCircleTool::~FilledCircleTool() = default;

void FilledCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
}

bool FilledCircleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos, shape);
}

bool FilledCircleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawFilledCircle(image, getColor(), start, calcRadius(start, end), shape);
}

void FilledCircleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawFilledRect(overlay, tool_overlay_color, centerRect(pos, shape));
}

void FilledCircleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("CENTER: ");
  status.append(start);
  status.append(" RADIUS: ");
  status.append(calcRadius(start, end));
}

StrokedRectangleTool::~StrokedRectangleTool() = default;

void StrokedRectangleTool::setThick(const int newThick) {
  thickness = newThick;
}

bool StrokedRectangleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos);
}

bool StrokedRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  return drawStrokedRect(image, getColor(), rect, thickness);
}

void StrokedRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, tool_overlay_color, pos);
}

void StrokedRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.appendLabeled(QRect{start, end}.normalized());
}

FilledRectangleTool::~FilledRectangleTool() = default;

bool FilledRectangleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos);
}

bool FilledRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  return drawFilledRect(image, getColor(), rect);
}

void FilledRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, tool_overlay_color, pos);
}

void FilledRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.appendLabeled(QRect{start, end}.normalized());
}
