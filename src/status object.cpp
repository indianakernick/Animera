//
//  status object.cpp
//  Animera
//
//  Created by Indiana Kernick on 12/11/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "status object.hpp"

#include "status msg.hpp"

StatusObject::StatusObject(QObject *parent)
  : QObject{parent} {}

void StatusObject::setScale(const int newScale) {
  scale = newScale;
  updateStatus();
}

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
  status.append("SCALE: ");
  status.append(scale);
  const std::size_t scaleSize = status.size();
  status.append(" CELLS: ");
  status.append(+frameCount, +layerCount);
  status.append(" CELL: ");
  status.append(+currPos.f, +currPos.l);
  if (selection.minL <= selection.maxL && selection.minF <= selection.maxF) {
    status.append(" SELECTION: ");
    status.append({
      QPoint{+selection.minF, +selection.minL},
      QPoint{+selection.maxF, +selection.maxL}
    });
  }
  Q_EMIT shouldShowPerm(status.get());
  Q_EMIT shouldShowApnd(status.get().substr(0, scaleSize));
}

#include "status object.moc"
