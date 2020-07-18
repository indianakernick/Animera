//
//  tool.cpp
//  Animera
//
//  Created by Indiana Kernick on 18/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "tool.hpp"

void ToolCtx::changeCelImage(const QRect rect) const {
  Q_EMIT celImageModified(rect);
}

void ToolCtx::changeCelImage(const QPoint point) const {
  changeCelImage(toRect(point));
}

void ToolCtx::changeOverlay(const QRect rect) const {
  Q_EMIT overlayModified(rect);
}

void ToolCtx::changeOverlay(const QPoint point) const {
  changeOverlay(toRect(point));
}

void ToolCtx::growCelImage(const QRect rect) const {
  Q_EMIT shouldGrowCelImage(rect);
}

void ToolCtx::shrinkCelImage(const QRect rect) const {
  Q_EMIT shouldShrinkCelImage(rect);
}

void ToolCtx::shrinkCelImage() const {
  Q_EMIT shouldShrinkCelImage(toRect(size));
}

PixelVar ToolCtx::selectColor(const ButtonType button) const {
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

void ToolCtx::clearStatus() const {
  Q_EMIT shouldShowNorm("");
}

void ToolCtx::finishChange() const {
  Q_EMIT changingAction();
}

void ToolCtx::lock() const {
  Q_EMIT shouldLock();
}

void ToolCtx::unlock() const {
  Q_EMIT shouldUnlock();
}

void Tool::setCtx(const ToolCtx *newCtx) {
  ctx = newCtx;
}

#include "tool.moc"
