//
//  transform tools.cpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "transform tools.hpp"

#include "cell.hpp"
#include <Graphics/fill.hpp>
#include <Graphics/mask.hpp>
#include "surface factory.hpp"
#include "graphics convert.hpp"
#include <Graphics/transform.hpp>

// @TODO do we need to know the difference between switching tools and switching cells?

void TranslateTool::attachCell() {
  //updateStatus();
}

void TranslateTool::detachCell() {
  ctx->clearStatus();
}

void TranslateTool::mouseLeave(const ToolLeaveEvent &) {
  ctx->clearStatus();
}

void TranslateTool::mouseDown(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary) return;
  lastPos = event.pos;
  drag = bool{*ctx->cell};
}

void TranslateTool::mouseMove(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary || !drag) {
    return updateStatus();
  }
  translate(event.pos - lastPos);
  updateStatus();
  lastPos = event.pos;
  ctx->emitChanges(ToolChanges::cell);
}

void TranslateTool::mouseUp(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary || !drag) return;
  translate(event.pos - lastPos);
  updateStatus();
  lastPos = event.pos;
  drag = false;
  ctx->emitChanges(ToolChanges::cell);
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
  if (!*ctx->cell) return;
  QPoint move = arrowToDir(event.key);
  if (move == QPoint{0, 0}) return;
  translate(move);
  updateStatus();
  ctx->emitChanges(ToolChanges::cell);
  ctx->finishChange();
}

void TranslateTool::translate(const QPoint move) {
  ctx->cell->image.setOffset(ctx->cell->image.offset() + move);
}

void TranslateTool::updateStatus() {
  ctx->showStatus(StatusMsg{}.appendLabeled(ctx->cell->image.offset()));
}

void FlipTool::attachCell() {
  flipX = flipY = false;
  //updateStatus();
}

void FlipTool::detachCell() {
  ctx->clearStatus();
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

void FlipTool::mouseMove(const ToolMouseEvent &) {
  updateStatus();
}

void FlipTool::keyPress(const ToolKeyEvent &event) {
  if (!*ctx->cell) return;
  if (flipXChanged(event.key, flipX)) {
    QImage &src = ctx->cell->image;
    QImage flipped{src.size(), src.format()};
    visitSurfaces(flipped, src, [](auto flipped, auto src) {
      gfx::flipHori(flipped, src);
    });
    QPoint offset = src.offset();
    offset.setX(ctx->size.width() - (offset.x() + src.width()));
    flipped.setOffset(offset);
    src = std::move(flipped);
  } else if (flipYChanged(event.key, flipY)) {
    QImage &src = ctx->cell->image;
    QImage flipped{src.size(), src.format()};
    visitSurfaces(flipped, src, [](auto flipped, auto src) {
      gfx::flipVert(flipped, src);
    });
    QPoint offset = src.offset();
    offset.setY(ctx->size.height() - (offset.y() + src.height()));
    flipped.setOffset(offset);
    src = std::move(flipped);
  } else {
    return;
  }
  updateStatus();
  ctx->emitChanges(ToolChanges::cell);
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
  //updateStatus();
}

void RotateTool::detachCell() {
  ctx->clearStatus();
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

void RotateTool::mouseMove(const ToolMouseEvent &) {
  updateStatus();
}

void RotateTool::keyPress(const ToolKeyEvent &event) {
  if (!*ctx->cell) return;
  const int rot = arrowToRot(event.key);
  if (rot == 0) return;
  angle = (angle + rot) & 3;
  QImage &src = ctx->cell->image;
  QImage rotated{src.size().transposed(), src.format()};
  visitSurfaces(rotated, src, [rot](auto rotated, auto src) {
    gfx::rotate(rotated, src, rot);
  });
  QPoint offset = src.offset();
  if (rot == 1) {
    offset = {ctx->size.height() - (offset.y() + src.height()), offset.x()};
  } else if (rot == 3) {
    offset = {offset.y(), ctx->size.width() - (offset.x() + src.width())};
  } else Q_UNREACHABLE();
  rotated.setOffset(offset);
  src = std::move(rotated);
  updateStatus();
  ctx->emitChanges(ToolChanges::cell);
  ctx->finishChange();
}

void RotateTool::updateStatus() {
  StatusMsg status;
  status.append("ANGLE: ");
  status.append(angle * 90);
  ctx->showStatus(status);
}
