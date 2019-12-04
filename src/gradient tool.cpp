//
//  gradient tool.cpp
//  Animera
//
//  Created by Indi Kernick on 6/10/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "gradient tool.hpp"

#include "painting.hpp"
#include "scope time.hpp"

void LinearGradientTool::mouseLeave(const ToolLeaveEvent &event) {
  SCOPE_TIME("LinearGradientTool::mouseLeave");
  
  ctx->clearStatus();
  drawSquarePoint(*ctx->overlay, 0, event.lastPos);
  ctx->changeOverlay(event.lastPos);
}

void LinearGradientTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("LinearGradientTool::mouseDown");

  if (event.button == ButtonType::secondary) {
    mode = opposite(mode);
  }
  
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (event.button != ButtonType::primary) {
    ctx->showStatus(status.appendLabeled(event.pos));
    return;
  }
  
  status.append("RECT: ");
  status.append({event.pos, QSize{1, 1}});
  ctx->showStatus(status);
  startPos = event.pos;
  if (ctx->colors.primary != 0 || ctx->colors.secondary != 0) {
    ctx->growCell(toRect(startPos));
  }
  cleanCell = *ctx->cell;
  const QPoint pos = ctx->cell->pos;
  drawSquarePoint(ctx->cell->img, ctx->colors.primary, startPos - pos);
  ctx->changeCell(startPos);
  ctx->lock();
}

void LinearGradientTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("LinearGradientTool::mouseMove");

  drawSquarePoint(*ctx->overlay, 0, event.lastPos);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (event.button != ButtonType::primary) {
    ctx->showStatus(status.appendLabeled(event.pos));
    ctx->changeOverlay(event.lastPos);
    ctx->changeOverlay(event.pos);
    return;
  }
  
  QRect rect = unite(startPos, event.pos);
  status.append("RECT: ");
  status.append(rect);
  ctx->showStatus(status);
  *ctx->cell = cleanCell;
  if (ctx->colors.primary != 0 || ctx->colors.secondary != 0) {
    ctx->growCell(rect);
  }
  drawGradient(rect, event.pos);
  ctx->changeCell(rect.united(toRect(event.lastPos)));
}

void LinearGradientTool::mouseUp(const ToolMouseEvent &event) {
  SCOPE_TIME("LinearGradientTool::mouseUp");
  
  if (event.button != ButtonType::primary) return;
  StatusMsg status;
  status.appendLabeled(mode);
  status.appendLabeled(event.pos);
  ctx->showStatus(status);
  ctx->unlock();
  if (ctx->colors.primary == 0 || ctx->colors.secondary == 0) {
    ctx->shrinkCell();
  }
  ctx->finishChange();
}

void LinearGradientTool::drawGradient(QRect rect, const QPoint endPos) {
  SCOPE_TIME("LinearGradientTool::drawGradient");
  
  QImage &img = ctx->cell->img;
  const QPoint pos = ctx->cell->pos;
  QRgb first = ctx->colors.primary;
  QRgb second = ctx->colors.secondary;
  if (mode == LineGradMode::hori) {
    if (startPos.x() > endPos.x()) {
      std::swap(first, second);
    }
    rect = rect.translated(-pos);
    drawHoriGradient(img, first, second, rect);
  } else if (mode == LineGradMode::vert) {
    if (startPos.y() > endPos.y()) {
      std::swap(first, second);
    }
    rect = rect.translated(-pos);
    drawVertGradient(img, first, second, rect);
  } else Q_UNREACHABLE();
}
