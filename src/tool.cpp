//
//  tool.cpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool.hpp"

void ToolCtx::emitChanges(const ToolChanges changes) const {
  if (changes == ToolChanges::cell || changes == ToolChanges::cell_overlay) {
    Q_EMIT cellModified(toRect(size));
  } else if (changes == ToolChanges::overlay) {
    Q_EMIT overlayModified(toRect(size));
  }
}

void ToolCtx::emitChanges(const bool drawn) const {
  if (drawn) {
    Q_EMIT cellModified(toRect(size));
  } else {
    Q_EMIT overlayModified(toRect(size));
  }
}

void ToolCtx::changeCell(const QRect rect) const {
  Q_EMIT cellModified(rect);
}

void ToolCtx::changeCell(const QPoint point) const {
  changeCell(toRect(point));
}

void ToolCtx::changeOverlay(const QRect rect) const {
  Q_EMIT overlayModified(rect);
}

void ToolCtx::changeOverlay(const QPoint point) const {
  changeOverlay(toRect(point));
}

void ToolCtx::growCell(const QRect rect) const {
  Q_EMIT cellRequested(rect);
}

QRgb ToolCtx::selectColor(const ButtonType button) const {
  switch (button) {
    case ButtonType::primary:   return colors.primary;
    case ButtonType::secondary: return colors.secondary;
    case ButtonType::erase:     return colors.erase;
    default: Q_UNREACHABLE();
  }
}

void ToolCtx::showStatus(const StatusMsg &status) const {
  Q_EMIT shouldShowNorm(status.get());
}

StatusMsg ToolCtx::showStatus() const {
  return StatusMsg{this};
}

void ToolCtx::clearStatus() const {
  Q_EMIT shouldShowNorm("");
}

void ToolCtx::finishChange() const {
  Q_EMIT changingAction();
}

void ToolCtx::lock() const {
  Q_EMIT lockRequested();
}

void ToolCtx::unlock() const {
  Q_EMIT unlockRequested();
}

void Tool::setCtx(const ToolCtx *newCtx) {
  ctx = newCtx;
}

#include "tool.moc"
