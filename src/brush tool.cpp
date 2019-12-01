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
  symPointOverlay(event.pos);
  symPointStatus(event.pos);
  ctx->growCell(symPointRect(event.pos));
  lastPos = event.pos;
  color = ctx->selectColor(event.button);
  symPoint(event.pos);
  ctx->lock();
}

void BrushTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("BrushTool::mouseMove");
  
  clearImage(*ctx->overlay);
  symPointOverlay(event.pos);
  symPointStatus(event.pos);
  if (event.button == ButtonType::none) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  ctx->growCell(symPointRect(event.pos));
  symLine({lastPos, event.pos});
  lastPos = event.pos;
}

void BrushTool::mouseUp(const ToolMouseEvent &event) {
  SCOPE_TIME("BrushTool::mouseUp");
  
  ctx->unlock();
  symPointStatus(event.pos);
  ctx->growCell(symPointRect(event.pos));
  symLine({lastPos, event.pos});
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

QPoint reflect(const QSize size, const QPoint point) {
  return {size.width() - point.x() - 1, size.height() - point.y() - 1};
}

}

template <typename Func>
void BrushTool::visit(Func func, const bool first) const {
  if (first)                               func(false, false);
  if (test_flag(mode, SymmetryMode::hori)) func(true,  false);
  if (test_flag(mode, SymmetryMode::vert)) func(false, true);
  if (test_flag(mode, SymmetryMode::both)) func(true,  true);
}

template <typename Func>
void BrushTool::visit(const QPoint point, Func func, const bool first) const {
  const QPoint refl = reflect(ctx->size, point);
  visit([func, refl, point](const bool hori, const bool vert) {
    const int x = hori ? refl.x() : point.x();
    const int y = vert ? refl.y() : point.y();
    func(QPoint{x, y});
  }, first);
}

template <typename Func>
void BrushTool::visit(const QLine line, Func func, const bool first) const {
  const QPoint reflP1 = reflect(ctx->size, line.p1());
  const QPoint reflP2 = reflect(ctx->size, line.p2());
  const QLine refl = {reflP1, reflP2};
  visit([func, refl, line](const bool hori, const bool vert) {
    const int x1 = hori ? refl.p1().x() : line.p1().x();
    const int y1 = vert ? refl.p1().y() : line.p1().y();
    const int x2 = hori ? refl.p2().x() : line.p2().x();
    const int y2 = vert ? refl.p2().y() : line.p2().y();
    func(QLine{x1, y1, x2, y2});
  }, first);
}

void BrushTool::symPointStatus(const QPoint point) {
  SCOPE_TIME("BrushTool::symPointStatus");
  
  StatusMsg status;
  status.appendLabeled(point);
  visit(point, [&status](const QPoint point) {
    status.append(' ');
    status.append(point);
  }, false);
  ctx->showStatus(status);
}

void BrushTool::symPointOverlay(const QPoint point) {
  SCOPE_TIME("BrushTool::symPointOverlay");
  
  visit(point, [this](const QPoint point) {
    drawRoundPoint(*ctx->overlay, tool_overlay_color, point, radius);
  });
}

void BrushTool::symPoint(const QPoint point) {
  SCOPE_TIME("BrushTool::symPoint");
  
  visit(point, [this](const QPoint point) {
    QImage &img = ctx->cell->img;
    const QPoint pos = ctx->cell->pos;
    const QRect rect = pointRect(point);
    ctx->emitChanges(drawRoundPoint(img, color, point - pos, radius), rect);
  });
}

void BrushTool::symLine(const QLine line) {
  SCOPE_TIME("BrushTool::symLine");
  
  visit(line, [this](const QLine line) {
    QImage &img = ctx->cell->img;
    const QPoint pos = ctx->cell->pos;
    const QRect rect = lineRect(line);
    ctx->emitChanges(drawLine(img, color, line.translated(-pos), radius), rect);
  });
}

QRect BrushTool::symPointRect(const QPoint point) const {
  SCOPE_TIME("BrushTool::symPointRect");
  
  QRect rect = pointRect(point);
  visit(point, [&rect, this](const QPoint point) {
    rect = rect.united(pointRect(point));
  }, false);
  return rect;
}

QRect BrushTool::pointRect(const QPoint point) const {
  return convert(gfx::circleRect(convert(point), radius));
}

QRect BrushTool::lineRect(const QLine line) const {
  return pointRect(line.p1()).united(pointRect(line.p2()));
}
