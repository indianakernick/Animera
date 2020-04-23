//
//  flood fill tool.cpp
//  Animera
//
//  Created by Indiana Kernick on 14/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "flood fill tool.hpp"

#include "cel.hpp"
#include "painting.hpp"
#include "composite.hpp"
#include "scope time.hpp"
#include <Graphics/draw.hpp>
#include "surface factory.hpp"
#include "graphics convert.hpp"

void FloodFillTool::mouseLeave(const ToolLeaveEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseLeave");
  
  ctx->clearStatus();
  drawSquarePoint(*ctx->overlay, 0, event.lastPos);
  ctx->changeOverlay(event.lastPos);
}

void FloodFillTool::mouseDown(const ToolMouseDownEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseDown");

  const QRgb fillColor = ctx->selectColor(event.button);
  const QRgb startColor = sampleCel(*ctx->cel, event.pos);
  if (fillColor == startColor) return;
  const QRect rect = toRect(ctx->size).intersected(ctx->cel->rect());
  if (startColor == 0) {
    if (rect.contains(event.pos)) {
      fillOpen(event.pos, fillColor);
    } else if (ctx->cel->isNull()) {
      ctx->growCel(toRect(ctx->size));
      drawFilledRect(ctx->cel->img, fillColor, toRect(ctx->size));
      ctx->changeCel(toRect(ctx->size));
    } else {
      ctx->growCel(toRect(ctx->size));
      const QRect fillRect = fill(toRect(ctx->size), event.pos, fillColor);
      ctx->shrinkCel(toRect(ctx->size));
      ctx->changeCel(fillRect);
    }
  } else {
    const QRect fillRect = fill(rect, event.pos, fillColor);
    if (fillColor == 0) ctx->shrinkCel(fillRect);
    ctx->changeCel(fillRect);
  }
  ctx->finishChange();
}

void FloodFillTool::mouseMove(const ToolMouseMoveEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseMove");

  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  drawSquarePoint(*ctx->overlay, 0, event.lastPos);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  ctx->changeOverlay(unite(event.lastPos, event.pos));
}

template <typename Pixel>
QRect FloodFillTool::fill(const QRect rect, const QPoint pos, const QRgb color) {
  SCOPE_TIME("FloodFillTool::fill");

  gfx::Surface surface = makeSurface<Pixel>(ctx->cel->img);
  surface = surface.view(convert(rect.translated(-ctx->cel->pos)));
  gfx::DrawFillPolicy policy{surface, static_cast<Pixel>(color)};
  return convert(gfx::floodFill(policy, convert(pos - rect.topLeft()))).translated(rect.topLeft());
}

QRect FloodFillTool::fill(const QRect rect, const QPoint pos, const QRgb color) {
  switch (ctx->format) {
    case Format::rgba:  return fill<PixelRgba> (rect, pos, color);
    case Format::index: return fill<PixelIndex>(rect, pos, color);
    case Format::gray:  return fill<PixelGray> (rect, pos, color);
  }
}

namespace {

template <auto Side>
bool sideSpill(const QRect fill, const QRect cel, const QRect canvas) {
  return (fill.*Side)() == (cel.*Side)() && (fill.*Side)() != (canvas.*Side)();
}

}

void FloodFillTool::fillOpen(const QPoint pos, const QRgb color) {
  const QRect canvasRect = toRect(ctx->size);
  const QRect celRect = canvasRect.intersected(ctx->cel->rect());
  QRect fillRect = fill(celRect, pos, color);
  
  const bool left   = sideSpill<&QRect::left>  (fillRect, celRect, canvasRect);
  const bool top    = sideSpill<&QRect::top>   (fillRect, celRect, canvasRect);
  const bool right  = sideSpill<&QRect::right> (fillRect, celRect, canvasRect);
  const bool bottom = sideSpill<&QRect::bottom>(fillRect, celRect, canvasRect);
  const bool any = left || top || right || bottom;
  
  if (any) ctx->growCel(canvasRect);
  
  if (left) {
    const QPoint leftPos = {celRect.left() - 1, celRect.top()};
    fillRect = fillRect.united(fill(canvasRect, leftPos, color));
  }
  if (top) {
    const QPoint topPos = {celRect.left(), celRect.top() - 1};
    fillRect = fillRect.united(fill(canvasRect, topPos, color));
  }
  if (right) {
    const QPoint rightPos = {celRect.right() + 1, celRect.top()};
    fillRect = fillRect.united(fill(canvasRect, rightPos, color));
  }
  if (bottom) {
    const QPoint bottomPos = {celRect.left(), celRect.bottom() + 1};
    fillRect = fillRect.united(fill(canvasRect, bottomPos, color));
  }
  
  if (any && fillRect != canvasRect) ctx->shrinkCel(canvasRect);
  
  ctx->changeCel(fillRect);
}
