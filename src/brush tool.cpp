//
//  brush tool.cpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "brush tool.hpp"

#include "cell.hpp"
#include "painting.hpp"
#include "scope time.hpp"
#include "graphics convert.hpp"

void BrushTool::mouseLeave(const ToolLeaveEvent &) {
  SCOPE_TIME("BrushTool::mouseLeave");
  
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void BrushTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("BrushTool::mouseDown");
  
  clearImage(*ctx->overlay);
  symPoint(*ctx->overlay, tool_overlay_color, event.pos);
  const QRect rect = symPointRect(event.pos);
  ctx->growCell(rect);
  symPointStatus(event.pos);
  lastPos = event.pos;
  color = ctx->selectColor(event.button);
  const QPoint pos = ctx->cell->pos;
  ctx->emitChanges(symPoint(ctx->cell->img, color, lastPos - pos), rect);
  ctx->lock();
}

void BrushTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("BrushTool::mouseMove");
  
  clearImage(*ctx->overlay);
  symPoint(*ctx->overlay, tool_overlay_color, event.pos);
  symPointStatus(event.pos);
  if (event.button == ButtonType::none) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  QRect rect = symPointRect(event.pos);
  ctx->growCell(rect);
  QImage &img = ctx->cell->img;
  const QPoint pos = ctx->cell->pos;
  rect = rect.united(symPointRect(lastPos));
  ctx->emitChanges(symLine(img, color, {lastPos - pos, event.pos - pos}), rect);
  lastPos = event.pos;
}

void BrushTool::mouseUp(const ToolMouseEvent &event) {
  SCOPE_TIME("BrushTool::mouseUp");
  
  ctx->unlock();
  symPointStatus(event.pos);
  QRect rect = symPointRect(event.pos);
  ctx->growCell(rect);
  QImage &img = ctx->cell->img;
  const QPoint pos = ctx->cell->pos;
  rect = rect.united(symPointRect(lastPos));
  ctx->emitChanges(symLine(img, color, {lastPos - pos, event.pos - pos}), rect);
  ctx->finishChange();
}

void BrushTool::setRadius(const int newRadius) {
  assert(brsh_radius.min <= newRadius && newRadius <= brsh_radius.max);
  radius = newRadius;
}

void BrushTool::setMode(const SymmetryMode newMode) {
  mode = newMode;
}

namespace {

QPoint reflectX(const QSize size, const QPoint point) {
  return {size.width() - point.x() - 1, point.y()};
}

QPoint reflectY(const QSize size, const QPoint point) {
  return {point.x(), size.height() - point.y() - 1};
}

QPoint reflectXY(const QSize size, const QPoint point) {
  return reflectX(size, reflectY(size, point));
}

}

void BrushTool::symPointStatus(const QPoint point) {
  SCOPE_TIME("BrushTool::symPointStatus");
  
  StatusMsg status;
  const QPoint refl = reflectXY(ctx->size, point);
  status.appendLabeled(point);
  if (test_flag(mode, SymmetryMode::hori)) {
    status.append(' ');
    status.append(QPoint{refl.x(), point.y()});
  }
  if (test_flag(mode, SymmetryMode::vert)) {
    status.append(' ');
    status.append(QPoint{point.x(), refl.y()});
  }
  if (test_flag(mode, SymmetryMode::both)) {
    status.append(' ');
    status.append(refl);
  }
  ctx->showStatus(status);
}

bool BrushTool::symPoint(QImage &img, const QRgb col, const QPoint point) {
  SCOPE_TIME("BrushTool::symPoint");
  
  const QPoint refl = reflectXY(img.size(), point);
  bool drawn = drawRoundPoint(img, col, point, radius);
  if (test_flag(mode, SymmetryMode::hori)) {
    drawn |= drawRoundPoint(img, col, {refl.x(), point.y()}, radius);
  }
  if (test_flag(mode, SymmetryMode::vert)) {
    drawn |= drawRoundPoint(img, col, {point.x(), refl.y()}, radius);
  }
  if (test_flag(mode, SymmetryMode::both)) {
    drawn |= drawRoundPoint(img, col, refl, radius);
  }
  return drawn;
}

bool BrushTool::symLine(QImage &img, const QRgb col, const QLine line) {
  SCOPE_TIME("BrushTool::symLine");
  
  const QSize size = ctx->size;
  bool drawn = drawLine(img, col, line, radius);
  if (test_flag(mode, SymmetryMode::hori)) {
    const QLine refl = {reflectX(size, line.p1()), reflectX(size, line.p2())};
    drawn |= drawLine(img, col, refl, radius);
  }
  if (test_flag(mode, SymmetryMode::vert)) {
    const QLine refl = {reflectY(size, line.p1()), reflectY(size, line.p2())};
    drawn |= drawLine(img, col, refl, radius);
  }
  if (test_flag(mode, SymmetryMode::both)) {
    const QLine refl = {reflectXY(size, line.p1()), reflectXY(size, line.p2())};
    drawn |= drawLine(img, col, refl, radius);
  }
  return drawn;
}

QRect BrushTool::symPointRect(const QPoint point) {
  SCOPE_TIME("BrushTool::symPointRect");
  
  const QPoint refl = reflectXY(ctx->size, point);
  QRect rect = convert(gfx::circleRect(convert(point), radius));
  if (test_flag(mode, SymmetryMode::hori)) {
    rect = rect.united(convert(gfx::circleRect({refl.x(), point.y()}, radius)));
  }
  if (test_flag(mode, SymmetryMode::vert)) {
    rect = rect.united(convert(gfx::circleRect({point.x(), refl.y()}, radius)));
  }
  if (test_flag(mode, SymmetryMode::both)) {
    rect = rect.united(convert(gfx::circleRect(convert(refl), radius)));
  }
  return rect;
}
