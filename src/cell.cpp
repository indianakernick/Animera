//
//  cell.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cell.hpp"

#include "serial.hpp"

/*
void serializeCell(QIODevice *dev, const Cell *cell) {
  assert(dev);
  if (cell) {
    cell->serialize(dev);
  } else {
    serializeBytes(dev, CellType::null);
  }
}

CellPtr deserializeCell(QIODevice *dev) {
  assert(dev);
  CellType type;
  deserializeBytes(dev, type);
  switch (type) {
    case CellType::null:
      return nullptr;
    case CellType::source:
      return std::make_unique<SourceCell>(dev);
    default: Q_UNREACHABLE();
  }
}
*/

Cell::Cell(const QSize size, const Format format, Palette *palette)
  : image{{size, getImageFormat(format)}, palette} {
  clearImage(image.data);
}

Cell::Cell(QIODevice *dev) {
  assert(dev);
  ::deserialize(dev, image);
}

void Cell::serialize(QIODevice *dev) const {
  assert(dev);
  //serializeBytes(dev, CellType::source);
  ::serialize(dev, image);
}

CellPtr Cell::clone() const {
  auto copy = std::make_unique<Cell>();
  copy->image = image;
  return copy;
}
