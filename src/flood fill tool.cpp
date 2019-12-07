//
//  flood fill tool.cpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "flood fill tool.hpp"

#include "cell.hpp"
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

void FloodFillTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseDown");

  const QRgb fillColor = ctx->selectColor(event.button);
  const QRgb startColor = sampleCell(*ctx->cell, event.pos);
  if (fillColor == startColor) return;
  const QRect rect = toRect(ctx->size).intersected(ctx->cell->rect());
  if (startColor == 0) {
    if (rect.contains(event.pos)) {
      fillOpen(event.pos, fillColor);
    } else if (ctx->cell->isNull()) {
      ctx->growCell(toRect(ctx->size));
      drawFilledRect(ctx->cell->img, fillColor, toRect(ctx->size));
      ctx->changeCell(toRect(ctx->size));
    } else {
      ctx->growCell(toRect(ctx->size));
      const QRect fillRect = fill(toRect(ctx->size), event.pos, fillColor);
      ctx->shrinkCell(toRect(ctx->size));
      ctx->changeCell(fillRect);
    }
  } else {
    const QRect fillRect = fill(rect, event.pos, fillColor);
    if (fillColor == 0) ctx->shrinkCell(fillRect);
    ctx->changeCell(fillRect);
  }
  ctx->finishChange();
}

void FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseMove");

  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  drawSquarePoint(*ctx->overlay, 0, event.lastPos);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  ctx->changeOverlay(unite(event.lastPos, event.pos));
}

template <typename Pixel>
QRect FloodFillTool::fill(const QRect rect, const QPoint pos, const QRgb color) {
  SCOPE_TIME("FloodFillTool::fill");

  gfx::Surface surface = makeSurface<Pixel>(ctx->cell->img);
  surface = surface.view(convert(rect.translated(-ctx->cell->pos)));
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
bool sideSpill(const QRect fill, const QRect cell, const QRect canvas) {
  return (fill.*Side)() == (cell.*Side)() && (fill.*Side)() != (canvas.*Side)();
}

}

void FloodFillTool::fillOpen(const QPoint pos, const QRgb color) {
  const QRect canvasRect = toRect(ctx->size);
  const QRect cellRect = canvasRect.intersected(ctx->cell->rect());
  QRect fillRect = fill(cellRect, pos, color);
  
  const bool left   = sideSpill<&QRect::left>  (fillRect, cellRect, canvasRect);
  const bool top    = sideSpill<&QRect::top>   (fillRect, cellRect, canvasRect);
  const bool right  = sideSpill<&QRect::right> (fillRect, cellRect, canvasRect);
  const bool bottom = sideSpill<&QRect::bottom>(fillRect, cellRect, canvasRect);
  const bool any = left || top || right || bottom;
  
  if (any) ctx->growCell(canvasRect);
  
  if (left) {
    const QPoint leftPos = {cellRect.left() - 1, cellRect.top()};
    fillRect = fillRect.united(fill(canvasRect, leftPos, color));
  }
  if (top) {
    const QPoint topPos = {cellRect.left(), cellRect.top() - 1};
    fillRect = fillRect.united(fill(canvasRect, topPos, color));
  }
  if (right) {
    const QPoint rightPos = {cellRect.right() + 1, cellRect.top()};
    fillRect = fillRect.united(fill(canvasRect, rightPos, color));
  }
  if (bottom) {
    const QPoint bottomPos = {cellRect.left(), cellRect.bottom() + 1};
    fillRect = fillRect.united(fill(canvasRect, bottomPos, color));
  }
  
  if (any && fillRect != canvasRect) ctx->shrinkCell(canvasRect);
  
  ctx->changeCell(fillRect);
}
