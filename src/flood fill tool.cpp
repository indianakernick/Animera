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

void FloodFillTool::detachCell() {
  ctx->clearStatus();
}

void FloodFillTool::mouseLeave(const ToolLeaveEvent &) {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void FloodFillTool::mouseDown(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  const QRgb color = ctx->selectColor(event.button);
  if (sampleCell(*ctx->cell, event.pos) == 0) {
    ctx->requireCell(toRect(ctx->size));
  }
  const QRect rect = toRect(ctx->size).intersected(ctx->cell->rect());
  ctx->emitChanges(drawFloodFill(ctx->cell->image, color, event.pos, rect));
  ctx->finishChange();
}

void FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  ctx->emitChanges(ToolChanges::overlay);
}
