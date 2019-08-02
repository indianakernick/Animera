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
    Q_EMIT cellModified();
  } else if (changes == ToolChanges::overlay) {
    Q_EMIT overlayModified();
  }
}

void ToolCtx::emitChanges(const bool drawn) const {
  if (drawn) {
    Q_EMIT cellModified();
  } else {
    Q_EMIT overlayModified();
  }
}

void ToolCtx::requireCell() const {
  if (!cell) Q_EMIT cellRequested();
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
  Q_EMIT shouldShowPerm(status.get());
}

void ToolCtx::clearStatus() const {
  Q_EMIT shouldShowPerm("");
}

void ToolCtx::finishChange() const {
  Q_EMIT changingAction();
}

void Tool::setCtx(const ToolCtx *newCtx) {
  ctx = newCtx;
}

#include "tool.moc"
