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
  ctx->changeOverlay(event.lastPos);
}

void FloodFillTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseDown");

  // TODO: avoid growing then shrinking
  // do the flood fill with a 1 pixel margin around the cell rect
  // check the top left pixel of the margin
  // fill rectangles as necessary
  // can the flood fill algorithm tell us if we're bumping into the edge?
  //   new fill policy to avoid actually allocating a bigger image

  QRect rect = toRect(ctx->size);
  bool grew = false;
  const QRgb color = ctx->selectColor(event.button);
  if (sampleCell(*ctx->cell, event.pos) == 0 && color != 0) {
    ctx->growCell(rect);
    grew = true;
  } else {
    rect = rect.intersected(ctx->cell->rect());
  }
  const QPoint pos = ctx->cell->pos;
  drawFloodFill(ctx->cell->img, color, event.pos - pos, rect.translated(-pos));
  if (grew || color == 0) ctx->shrinkCell();
  ctx->changeCell(rect);
  ctx->finishChange();
}

void FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("FloodFillTool::mouseMove");

  ctx->showStatus(StatusMsg{}.appendLabeled(event.pos));
  drawSquarePoint(*ctx->overlay, 0, event.lastPos);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  ctx->changeOverlay(event.lastPos);
  ctx->changeOverlay(event.pos);
}
