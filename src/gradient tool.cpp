//
//  gradient tool.cpp
//  Animera
//
//  Created by Indiana Kernick on 6/10/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "gradient tool.hpp"

#include "painting.hpp"
#include "scope time.hpp"
#include "config colors.hpp"

void LinearGradientTool::mouseLeave(const ToolLeaveEvent &event) {
  SCOPE_TIME("LinearGradientTool::mouseLeave");
  
  ctx->clearStatus();
  drawSquarePoint(*ctx->overlay, {}, event.lastPos);
  ctx->changeOverlay(event.lastPos);
}

void LinearGradientTool::mouseDown(const ToolMouseDownEvent &event) {
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
  
  status.append("Rect: ");
  status.append({event.pos, QSize{1, 1}});
  ctx->showStatus(status);
  startPos = event.pos;
  if (!ctx->colors.primary.zero() || !ctx->colors.secondary.zero()) {
    ctx->growCel(toRect(startPos));
  }
  cleanCel = *ctx->cel;
  const QPoint pos = ctx->cel->pos;
  drawSquarePoint(ctx->cel->img, ctx->colors.primary, startPos - pos);
  ctx->changeCel(startPos);
  ctx->lock();
}

void LinearGradientTool::mouseMove(const ToolMouseMoveEvent &event) {
  SCOPE_TIME("LinearGradientTool::mouseMove");

  drawSquarePoint(*ctx->overlay, {}, event.lastPos);
  drawSquarePoint(*ctx->overlay, PixelVar{tool_overlay_color}, event.pos);
  
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (event.button != ButtonType::primary) {
    ctx->showStatus(status.appendLabeled(event.pos));
    ctx->changeOverlay(unite(event.lastPos, event.pos));
    return;
  }
  
  QRect rect = unite(startPos, event.pos);
  status.append("Rect: ");
  status.append(rect);
  ctx->showStatus(status);
  *ctx->cel = cleanCel;
  if (!ctx->colors.primary.zero() || !ctx->colors.secondary.zero()) {
    ctx->growCel(rect);
  }
  drawGradient(rect, event.pos);
  ctx->changeCel(rect.united(toRect(event.lastPos)));
}

void LinearGradientTool::mouseUp(const ToolMouseUpEvent &event) {
  SCOPE_TIME("LinearGradientTool::mouseUp");
  
  if (event.button != ButtonType::primary) return;
  StatusMsg status;
  status.appendLabeled(mode);
  status.appendLabeled(event.pos);
  ctx->showStatus(status);
  ctx->unlock();
  
  const bool primaryZero = ctx->colors.primary.zero();
  const bool secondaryZero = ctx->colors.secondary.zero();
  const QRect rect = unite(startPos, event.pos);
  if (primaryZero && secondaryZero) {
    ctx->shrinkCel(rect);
  } else if (primaryZero || secondaryZero) {
    const QPoint pos = primaryZero ? startPos : event.pos;
    if (mode == LineGradMode::hori) {
      ctx->shrinkCel({pos.x(), rect.top(), 1, rect.height()});
    } else if (mode == LineGradMode::vert) {
      ctx->shrinkCel({rect.left(), pos.y(), rect.width(), 1});
    } else Q_UNREACHABLE();
  }
  
  ctx->finishChange();
}

void LinearGradientTool::drawGradient(QRect rect, const QPoint endPos) {
  SCOPE_TIME("LinearGradientTool::drawGradient");
  
  QImage &img = ctx->cel->img;
  const QPoint pos = ctx->cel->pos;
  PixelVar first = ctx->colors.primary;
  PixelVar second = ctx->colors.secondary;
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
