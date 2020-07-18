//
//  brush tool.cpp
//  Animera
//
//  Created by Indiana Kernick on 14/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "brush tool.hpp"

#include "cel.hpp"
#include "painting.hpp"
#include "scope time.hpp"
#include "config colors.hpp"
#include "graphics convert.hpp"

void BrushTool::mouseLeave(const ToolLeaveEvent &event) {
  SCOPE_TIME("BrushTool::mouseLeave");
  
  ctx->clearStatus();
  symPointOverlay(event.lastPos, 0);
  symChangeOverlay({event.lastPos, event.lastPos});
}

void BrushTool::mouseDown(const ToolMouseDownEvent &event) {
  SCOPE_TIME("BrushTool::mouseDown");
  
  color = ctx->selectColor(event.button);
  const QRect rect = symPointRect(event.pos);
  if (color.zero()) {
    bounds = rect;
  } else {
    ctx->growCelImage(rect);
  }
  symPoint(event.pos);
  ctx->lock();
}

void BrushTool::mouseMove(const ToolMouseMoveEvent &event) {
  SCOPE_TIME("BrushTool::mouseMove");
  
  symPointStatus(event.pos);
  symPointOverlay(event.lastPos, 0);
  symPointOverlay(event.pos, tool_overlay_color);
  if (event.button == ButtonType::none) {
    symChangeOverlay({event.lastPos, event.pos});
    return;
  }
  const QRect rect = symPointRect(event.pos);
  if (color.zero()) {
    bounds = bounds.united(rect);
  } else {
    ctx->growCelImage(rect);
  }
  symLine({event.lastPos, event.pos});
}

void BrushTool::mouseUp(const ToolMouseUpEvent &) {
  SCOPE_TIME("BrushTool::mouseUp");
  
  ctx->unlock();
  if (color.zero()) ctx->shrinkCelImage(bounds);
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

void BrushTool::symPointOverlay(const QPoint point, const QRgb col) {
  SCOPE_TIME("BrushTool::symPointOverlay");
  
  visit(point, [this, col](const QPoint point) {
    drawRoundPoint(*ctx->overlay, PixelVar{col}, point, radius);
  });
}

void BrushTool::symChangeOverlay(const QLine line) {
  SCOPE_TIME("BrushTool::symChangeOverlay");
  
  visit(line, [this](const QLine line) {
    ctx->changeOverlay(lineRect(line));
  });
}

void BrushTool::symPoint(const QPoint point) {
  SCOPE_TIME("BrushTool::symPoint");
  
  visit(point, [this](const QPoint point) {
    QImage &img = ctx->cel->img;
    const QPoint pos = ctx->cel->pos;
    const QRect rect = pointRect(point);
    drawRoundPoint(img, color, point - pos, radius);
    ctx->changeCelImage(rect);
  });
}

void BrushTool::symLine(const QLine line) {
  SCOPE_TIME("BrushTool::symLine");
  
  visit(line, [this](const QLine line) {
    QImage &img = ctx->cel->img;
    const QPoint pos = ctx->cel->pos;
    drawLine(img, color, line.translated(-pos), radius);
    ctx->changeCelImage(lineRect(line));
  });
}

QRect BrushTool::symPointRect(const QPoint point) const {
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
