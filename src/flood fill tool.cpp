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

void FloodFillTool::mouseLeave(const ToolLeaveEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseLeave");
  
  ctx->clearStatus();
  drawSquarePoint(*ctx->overlay, 0, event.lastPos);
  ctx->changeOverlay(toRect(event.lastPos));
}

void FloodFillTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseDown");

  drawSquarePoint(*ctx->overlay, 0, event.lastPos);
  ctx->changeOverlay(toRect(event.lastPos));
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  QRect rect = toRect(ctx->size);
  if (sampleCell(*ctx->cell, event.pos) == 0) {
    ctx->growCell(rect);
  } else {
    rect = rect.intersected(ctx->cell->rect());
  }
  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  const QRgb color = ctx->selectColor(event.button);
  const QPoint pos = ctx->cell->pos;
  drawFloodFill(ctx->cell->img, color, event.pos - pos, rect.translated(-pos));
  ctx->changeCell(rect);
  ctx->finishChange();
}

void FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseMove");

  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  drawSquarePoint(*ctx->overlay, 0, event.lastPos);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  ctx->changeOverlay(unite(event.lastPos, event.pos));
}
