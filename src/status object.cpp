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
  if (scale == newScale) return;
  scale = newScale;
  updateStatus(Updated::scale);
}

void StatusObject::setPos(const CellPos newPos) {
  if (pos.l == newPos.l && pos.f == newPos.f) return;
  pos = newPos;
  updateStatus(Updated::cell);
}

void StatusObject::setSelection(const CellRect newSelection) {
  if (
    selection.minL == newSelection.minL &&
    selection.minF == newSelection.minF &&
    selection.maxL == newSelection.maxL &&
    selection.maxF == newSelection.maxF
  ) return;
  selection = newSelection;
  updateStatus(Updated::selection);
}

void StatusObject::setFrameCount(const FrameIdx newFrameCount) {
  if (frameCount == newFrameCount) return;
  frameCount = newFrameCount;
  updateStatus(Updated::timeline);
}

void StatusObject::setLayerCount(const LayerIdx newLayerCount) {
  if (layerCount == newLayerCount) return;
  layerCount = newLayerCount;
  updateStatus(Updated::timeline);
}

void StatusObject::updateStatus(const Updated updated) {
  StatusMsg status;
  
  status.append("Scale: ");
  status.append(scale);
  const std::size_t scaleSize = status.size();
  
  status.append(" Timeline: ");
  status.append(+frameCount, +layerCount);
  const std::size_t timelineSize = status.size();
  
  status.append(" Cell: ");
  status.append(+pos.f, +pos.l);
  const std::size_t cellSize = status.size();
  
  if (selection.minL <= selection.maxL && selection.minF <= selection.maxF) {
    status.append(" Selection: ");
    status.append({
      QPoint{+selection.minF, +selection.minL},
      QPoint{+selection.maxF, +selection.maxL}
    });
  }
  
  std::string_view append;
  switch (updated) {
    case Updated::scale:
      append = status.get().substr(0, scaleSize);
      break;
    case Updated::timeline:
      append = status.get().substr(scaleSize + 1, timelineSize - scaleSize - 1);
      break;
    case Updated::cell:
      append = status.get().substr(timelineSize + 1, cellSize - timelineSize - 1);
      break;
    case Updated::selection:
      // selection can't be changed without moving mouse out of editor
      break;
  }
  
  Q_EMIT shouldShowPerm(status.get());
  if (!append.empty()) {
    Q_EMIT shouldShowApnd(append);
  }
}

#include "status object.moc"
