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

void FloodFillTool::mouseLeave(const ToolLeaveEvent &) {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void FloodFillTool::mouseDown(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  QRect rect = toRect(ctx->size);
  if (sampleCell(*ctx->cell, event.pos) == 0) {
    ctx->growCell(rect);
  } else {
    rect = rect.intersected(ctx->cell->rect());
  }
  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  const QRgb color = ctx->selectColor(event.button);
  QImage &img = ctx->cell->img;
  const QPoint pos = ctx->cell->pos;
  rect.translate(-pos);
  ctx->emitChanges(drawFloodFill(img, color, event.pos - pos, rect));
  ctx->finishChange();
}

void FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  ctx->emitChanges(ToolChanges::overlay);
}
