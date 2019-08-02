//
//  transform tools.cpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "transform tools.hpp"

#include "cell.hpp"
#include "masking.hpp"
#include "transform.hpp"
#include "surface factory.hpp"

// @TODO do we need to know the difference between switching tools and switching cells?

void TranslateTool::attachCell() {
  if (!compatible(cleanImage, ctx->cell->image)) {
    cleanImage = makeCompatible(ctx->cell->image);
  }
  copyImage(cleanImage, ctx->cell->image);
  pos = {0, 0};
  //updateStatus();
}

void TranslateTool::detachCell() {
  ctx->clearStatus();
}

void TranslateTool::mouseLeave(const ToolLeaveEvent &) {
  ctx->clearStatus();
}

void TranslateTool::mouseDown(const ToolMouseEvent &event) {
  ctx->requireCell();
  if (event.button != ButtonType::primary) return;
  lastPos = event.pos;
  drag = true;
}

void TranslateTool::mouseMove(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary || !drag) {
    return updateStatus();
  }
  translate(event.pos - lastPos, ctx->colors.erase);
  updateStatus();
  lastPos = event.pos;
  ctx->emitChanges(ToolChanges::cell);
}

void TranslateTool::mouseUp(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary || !drag) return;
  translate(event.pos - lastPos, ctx->colors.erase);
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
  QPoint move = arrowToDir(event.key);
  if (move == QPoint{0, 0}) return;
  ctx->requireCell();
  translate(move, ctx->colors.erase);
  updateStatus();
  ctx->emitChanges(ToolChanges::cell);
  ctx->finishChange();
}

void TranslateTool::translate(const QPoint move, const QRgb eraseColor) {
  pos += move;
  updateSourceImage(eraseColor);
}

void TranslateTool::updateSourceImage(const QRgb eraseColor) {
  visitSurfaces(ctx->cell->image, cleanImage, [this, eraseColor](auto src, auto clean) {
    src.overFill(eraseColor);
    copyRegion(src, clean, pos);
  });
}

void TranslateTool::updateStatus() {
  ctx->showStatus(StatusMsg{}.appendLabeled(pos));
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
  if (flipXChanged(event.key, flipX)) {
    ctx->requireCell();
    QImage &src = ctx->cell->image;
    QImage flipped{src.size(), src.format()};
    visitSurfaces(flipped, src, [](auto flipped, auto src) {
      flipHori(flipped, src);
    });
    src = flipped;
  } else if (flipYChanged(event.key, flipY)) {
    ctx->requireCell();
    QImage &src = ctx->cell->image;
    QImage flipped{src.size(), src.format()};
    visitSurfaces(flipped, src, [](auto flipped, auto src) {
      flipVert(flipped, src);
    });
    src = flipped;
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
  const QSize size = ctx->cell->image.size();
  square = size.width() == size.height();
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

quint8 arrowToRot(const Qt::Key key) {
  switch (key) {
    case key_rot_cw_a:
    case key_rot_cw_b: return 1;
    case key_rot_ccw_a:
    case key_rot_ccw_b: return 3;
    default: return 0;
  }
}

}

void RotateTool::mouseMove(const ToolMouseEvent &) {
  updateStatus();
}

void RotateTool::keyPress(const ToolKeyEvent &event) {
  const quint8 rot = arrowToRot(event.key);
  if (square && rot) {
    ctx->requireCell();
    angle = (angle + rot) & 3;
    QImage &src = ctx->cell->image;
    QImage rotated{src.size(), src.format()};
    visitSurfaces(rotated, src, [rot](auto rotated, auto src) {
      rotate(rotated, src, rot);
    });
    src = rotated;
    updateStatus();
    ctx->emitChanges(ToolChanges::cell);
    ctx->finishChange();
  }
}

void RotateTool::updateStatus() {
  StatusMsg status;
  if (square) {
    status.append("ANGLE: ");
    status.append(angle * 90);
  } else {
    // I think this is a sensible limitation
    // Sprites for games are square 99% of the time
    status.append("Only square sprites can be rotated");
  }
  ctx->showStatus(status);
}
