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
