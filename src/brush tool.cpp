//
//  brush tool.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "brush tool.hpp"

#include "cell.hpp"
#include "painting.hpp"

void BrushTool::detachCell() {
  ctx->clearStatus();
}

void BrushTool::mouseLeave(const ToolLeaveEvent &) {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void BrushTool::mouseDown(const ToolMouseEvent &event) {
  ctx->requireCell();
  clearImage(*ctx->overlay);
  symPoint(*ctx->overlay, tool_overlay_color, event.pos);
  symPointStatus(event.pos);
  lastPos = event.pos;
  color = ctx->selectColor(event.button);
  ctx->emitChanges(symPoint(ctx->cell->image, color, lastPos));
}

void BrushTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  symPoint(*ctx->overlay, tool_overlay_color, event.pos);
  symPointStatus(event.pos);
  if (event.button == ButtonType::none) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  ctx->emitChanges(symLine(ctx->cell->image, color, {lastPos, event.pos}));
  lastPos = event.pos;
}

void BrushTool::mouseUp(const ToolMouseEvent &event) {
  symPointStatus(event.pos);
  ctx->emitChanges(symLine(ctx->cell->image, color, {lastPos, event.pos}));
  ctx->finishChange();
}

void BrushTool::setRadius(const int newRadius) {
  assert(brsh_min_radius <= newRadius && newRadius <= brsh_max_radius);
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
  StatusMsg status;
  const QSize size = ctx->overlay->size();
  const QPoint refl = reflectXY(size, point);
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
  const QSize size = img.size();
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
