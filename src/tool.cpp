//
//  tool.cpp
//  Animera
//
//  Created by Indiana Kernick on 18/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "tool.hpp"

void ToolCtx::changeCel(const QRect rect) const {
  Q_EMIT celModified(rect);
}

void ToolCtx::changeCel(const QPoint point) const {
  changeCel(toRect(point));
}

void ToolCtx::changeOverlay(const QRect rect) const {
  Q_EMIT overlayModified(rect);
}

void ToolCtx::changeOverlay(const QPoint point) const {
  changeOverlay(toRect(point));
}

void ToolCtx::growCel(const QRect rect) const {
  Q_EMIT growRequested(rect);
}

void ToolCtx::shrinkCel(const QRect rect) const {
  Q_EMIT shrinkRequested(rect);
}

void ToolCtx::shrinkCel() const {
  Q_EMIT shrinkRequested(toRect(size));
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
