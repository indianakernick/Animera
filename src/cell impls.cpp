//
//  cell impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cell impls.hpp"

#include "serial.hpp"

namespace {

const Cell *findNonDup(const Cell *cell) {
  while (auto *dup = dynamic_cast<const DuplicateCell *>(cell)) {
    cell = dup->source;
  }
  return cell;
}

const Cell *findSrc(const Cell *cell) {
  while (cell) {
    if (auto *dup = dynamic_cast<const DuplicateCell *>(cell)) {
      cell = dup->source;
    } else if (auto *trans = dynamic_cast<const TransformCell *>(cell)) {
      cell = trans->source;
    } else {
      break;
    }
  }
  return cell;
}

Transform findTransform(const Cell *cell) {
  cell = findNonDup(cell);
  if (auto *src = dynamic_cast<const SourceCell *>(cell)) {
    return src->image.xform;
  } else if (auto *trans = dynamic_cast<const TransformCell *>(cell)) {
    return trans->xform;
  } else {
    return {};
  }
}

}

void serializeCell(QIODevice *dev, const Cell *cell) {
  assert(dev);
  if (cell) {
    cell->serialize(dev);
  } else {
    serialize(dev, CellType::null);
  }
}

CellPtr deserializeCell(QIODevice *dev) {
  assert(dev);
  CellType type;
  deserialize(dev, type);
  switch (type) {
    case CellType::null:
      return nullptr;
    case CellType::source:
      return std::make_unique<SourceCell>(dev);
    case CellType::duplicate:
      return std::make_unique<DuplicateCell>(dev);
    case CellType::transform:
      return std::make_unique<TransformCell>(dev);
    default: Q_UNREACHABLE();
  }
}

SourceCell::SourceCell(const QSize size, const Format format)
  : image{{size, getImageFormat(format)}, {}} {
  image.data.fill(0);
}

SourceCell::SourceCell(QIODevice *dev) {
  assert(dev);
  ::deserialize(dev, image);
}

void SourceCell::serialize(QIODevice *dev) const {
  assert(dev);
  ::serialize(dev, CellType::source);
  ::serialize(dev, image);
}

Image SourceCell::outputImage() const {
  return image;
}

void SourceCell::updateInput(const Cell *) {}

CellPtr SourceCell::clone() const {
  auto copy = std::make_unique<SourceCell>();
  copy->image = image;
  return copy;
}

DuplicateCell::DuplicateCell(const Cell *input) {
  updateInput(input);
}

DuplicateCell::DuplicateCell(QIODevice *) {
  // nothing to do
}

void DuplicateCell::serialize(QIODevice *dev) const {
  assert(dev);
  ::serialize(dev, CellType::duplicate);
}

Image DuplicateCell::outputImage() const {
  return source ? source->outputImage() : Image{};
}

void DuplicateCell::updateInput(const Cell *input) {
  source = findNonDup(input);
}

CellPtr DuplicateCell::clone() const {
  return std::make_unique<DuplicateCell>();
}

TransformCell::TransformCell(const Cell *input) {
  updateInput(input);
  xform = findTransform(input);
}

TransformCell::TransformCell(QIODevice *dev) {
  assert(dev);
  ::deserialize(dev, xform);
}

void TransformCell::serialize(QIODevice *dev) const {
  assert(dev);
  ::serialize(dev, CellType::transform);
  ::serialize(dev, xform);
}

Image TransformCell::outputImage() const {
  return source ? Image{source->outputImage().data, xform} : Image{};
}

void TransformCell::updateInput(const Cell *input) {
  source = findSrc(input);
}

CellPtr TransformCell::clone() const {
  auto copy = std::make_unique<TransformCell>();
  copy->xform = xform;
  return copy;
}
