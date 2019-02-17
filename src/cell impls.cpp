//
//  cell impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cell impls.hpp"

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

template <typename T>
void serialize(QIODevice *dev, const T &data) {
  assert(dev->isWritable());
  dev->write(reinterpret_cast<const char *>(&data), sizeof(T));
}

void serialize(QIODevice *dev, const Transform &xform) {
  assert(dev);
  serialize(dev, xform.posX);
  serialize(dev, xform.posY);
  serialize(dev, xform.angle);
  serialize(dev, xform.flipX);
  serialize(dev, xform.flipY);
}

}

SourceCell::SourceCell(const QSize size, const Format format)
  : image{{size, getImageFormat(format)}, {}} {}

Image SourceCell::outputImage() const {
  return image;
}

void SourceCell::updateInput(const Cell *) {}

CellPtr SourceCell::clone() const {
  auto copy = std::make_unique<SourceCell>();
  copy->image = image;
  return copy;
}

void SourceCell::serialize(QIODevice *dev) const {
  assert(dev);
  ::serialize(dev, CellType::source);
  ::serialize(dev, image.xform);
  assert(!image.data.isNull());
  assert(dev->isWritable());
  image.data.save(dev, "png");
}

DuplicateCell::DuplicateCell(const Cell *input) {
  updateInput(input);
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

void DuplicateCell::serialize(QIODevice *dev) const {
  assert(dev);
  ::serialize(dev, CellType::duplicate);
}

TransformCell::TransformCell(const Cell *input) {
  updateInput(input);
  xform = findTransform(input);
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

void TransformCell::serialize(QIODevice *dev) const {
  assert(dev);
  ::serialize(dev, CellType::transform);
  ::serialize(dev, xform);
}
