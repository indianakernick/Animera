//
//  transform tools.cpp
//  Animera
//
//  Created by Indiana Kernick on 14/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "transform tools.hpp"

#include "cell.hpp"
#include <Graphics/fill.hpp>
#include <Graphics/mask.hpp>
#include "surface factory.hpp"
#include "graphics convert.hpp"
#include <Graphics/transform.hpp>

void TranslateTool::mouseLeave(const ToolLeaveEvent &) {
  ctx->clearStatus();
}

void TranslateTool::mouseDown(const ToolMouseDownEvent &event) {
  if (event.button != ButtonType::primary) return;
  drag = !ctx->cell->isNull();
  if (drag) ctx->lock();
}

void TranslateTool::mouseMove(const ToolMouseMoveEvent &event) {
  if (event.button != ButtonType::primary || !drag) {
    return updateStatus();
  }
  const QPoint move = event.pos - event.lastPos;
  if (move == QPoint{0, 0}) return;
  translate(move);
  updateStatus();
}

void TranslateTool::mouseUp(const ToolMouseUpEvent &event) {
  if (event.button != ButtonType::primary || !drag) return;
  drag = false;
  ctx->unlock();
  ctx->finishChange();
}

namespace {

QPoint arrowToDir(const Qt::Key key) {
  switch (key) {
    case key_mov_up:    return {0, -1};
    case key_mov_right: return {1, 0};
    case key_mov_down:  return {0, 1};
    case key_mov_left:  return {-1, 0};
    default:            return {0, 0};
  }
}

}

void TranslateTool::keyPress(const ToolKeyEvent &event) {
  if (ctx->cell->isNull()) return;
  QPoint move = arrowToDir(event.key);
  if (move == QPoint{0, 0}) return;
  translate(move);
  updateStatus();
  ctx->finishChange();
}

void TranslateTool::translate(const QPoint move) {
  const QRect rect = ctx->cell->rect();
  ctx->cell->pos += move;
  ctx->changeCell(rect.united(rect.translated(move)));
}

void TranslateTool::updateStatus() {
  ctx->showStatus(StatusMsg{}.appendLabeled(ctx->cell->pos));
}

void FlipTool::attachCell() {
  flipX = flipY = false;
}

void FlipTool::mouseLeave(const ToolLeaveEvent &) {
  ctx->clearStatus();
}

namespace {

bool flipXChanged(const Qt::Key key, bool &flipX) {
  switch (key) {
    case key_flp_on_x: return !std::exchange(flipX, true);
    case key_flp_off_x: return std::exchange(flipX, false);
    default: return false;
  }
}

bool flipYChanged(const Qt::Key key, bool &flipY) {
  switch (key) {
    case key_flp_on_y: return !std::exchange(flipY, true);
    case key_flp_off_y: return std::exchange(flipY, false);
    default: return false;
  }
}

}

void FlipTool::mouseMove(const ToolMouseMoveEvent &) {
  updateStatus();
}

void FlipTool::keyPress(const ToolKeyEvent &event) {
  if (ctx->cell->isNull()) return;
  const QRect rect = ctx->cell->rect();
  if (flipXChanged(event.key, flipX)) {
    QImage &src = ctx->cell->img;
    QImage flipped{src.size(), src.format()};
    visitSurfaces(flipped, src, [](auto flipped, auto src) {
      gfx::flipHori(flipped, src);
    });
    ctx->cell->pos.setX(ctx->size.width() - (ctx->cell->pos.x() + src.width()));
    src = std::move(flipped);
  } else if (flipYChanged(event.key, flipY)) {
    QImage &src = ctx->cell->img;
    QImage flipped{src.size(), src.format()};
    visitSurfaces(flipped, src, [](auto flipped, auto src) {
      gfx::flipVert(flipped, src);
    });
    ctx->cell->pos.setY(ctx->size.height() - (ctx->cell->pos.y() + src.height()));
    src = std::move(flipped);
  } else {
    return;
  }
  updateStatus();
  ctx->changeCell(rect.united({ctx->cell->pos, rect.size()}));
  ctx->finishChange();
}

void FlipTool::updateStatus() {
  StatusMsg status;
  status.append("X: ");
  status.append(flipX);
  status.append(" Y: ");
  status.append(flipY);
  ctx->showStatus(status);
}

void RotateTool::attachCell() {
  angle = 0;
}

void RotateTool::mouseLeave(const ToolLeaveEvent &) {
  ctx->clearStatus();
}

namespace {

int arrowToRot(const Qt::Key key) {
  switch (key) {
    case key_rot_cw_a:
    case key_rot_cw_b:  return 1;
    case key_rot_ccw_a:
    case key_rot_ccw_b: return 3;
    default:            return 0;
  }
}

}

void RotateTool::mouseMove(const ToolMouseMoveEvent &) {
  updateStatus();
}

void RotateTool::keyPress(const ToolKeyEvent &event) {
  if (ctx->cell->isNull()) return;
  const int rot = arrowToRot(event.key);
  if (rot == 0) return;
  const QRect rect = ctx->cell->rect();
  angle = (angle + rot) & 3;
  QImage &src = ctx->cell->img;
  QImage rotated{src.size().transposed(), src.format()};
  visitSurfaces(rotated, src, [rot](auto rotated, auto src) {
    gfx::rotate(rotated, src, rot);
  });
  QSize size = ctx->size;
  if (size.width() % 2 > size.height() % 2) {
    size = {size.width() - 1, size.height()};
  } else if (size.width() % 2 < size.height() % 2) {
    size = {size.width(), size.height() - 1};
  }
  const QPoint pos = ctx->cell->pos;
  if (rot == 1) {
    ctx->cell->pos = {
      size.width() / 2 + (size.height() + 1) / 2 - pos.y() - src.height(),
      (size.height() + 1) / 2 - (size.width() + 1) / 2 + pos.x()
    };
  } else if (rot == 3) {
    ctx->cell->pos = {
      (size.width() + 1) / 2 - (size.height() + 1) / 2 + pos.y(),
      size.height() / 2 + (size.width() + 1) / 2 - pos.x() - src.width()
    };
  } else Q_UNREACHABLE();
  src = std::move(rotated);
  updateStatus();
  ctx->changeCell(rect.united(ctx->cell->rect()));
  ctx->finishChange();
}

void RotateTool::updateStatus() {
  StatusMsg status;
  status.append("Angle: ");
  status.append(angle * 90);
  ctx->showStatus(status);
}
