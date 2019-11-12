//
//  timeline status object.cpp
//  Animera
//
//  Created by Indi Kernick on 12/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline status object.hpp"

#include "status msg.hpp"

StatusObject::StatusObject(QObject *parent)
  : QObject{parent} {}

void StatusObject::setCurrPos(const CellPos newCurrPos) {
  currPos = newCurrPos;
  updateStatus();
}

void StatusObject::setSelection(const CellRect newSelection) {
  selection = newSelection;
  updateStatus();
}

void StatusObject::setFrameCount(const FrameIdx newFrameCount) {
  frameCount = newFrameCount;
  updateStatus();
}

void StatusObject::setLayerCount(const LayerIdx newLayerCount) {
  layerCount = newLayerCount;
  updateStatus();
}

void StatusObject::updateStatus() {
  StatusMsg status;
  status.append("CELLS: ");
  status.append(QSize{+frameCount, +layerCount});
  status.append(" CELL: ");
  status.append(QPoint{+currPos.f, +currPos.l});
  if (selection.minL <= selection.maxL && selection.minF <= selection.maxF) {
    status.append(" SELECTION: ");
    status.append({
      QPoint{+selection.minF, +selection.minL},
      QPoint{+selection.maxF, +selection.maxL}
    });
  }
  Q_EMIT shouldShowPerm(status.get());
}

#include "timeline status object.moc"
