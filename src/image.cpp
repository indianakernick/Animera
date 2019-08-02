//
//  image.cpp
//  Animera
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "image.hpp"

#include "surface factory.hpp"

bool compatible(const QImage &a, const QImage &b) {
  return a.size() == b.size() && a.format() == b.format();
}

QImage makeCompatible(const QImage &img) {
  return QImage{img.size(), img.format()};
}

QImage makeMask(const QSize size) {
  return QImage{size, mask_format};
}

void copyImage(QImage &dst, const QImage &src) {
  assert(compatible(dst, src));
  dst.detach();
  std::memcpy(dst.bits(), src.constBits(), dst.sizeInBytes());
}

void clearImage(QImage &dst) {
  dst.detach();
  std::memset(dst.bits(), 0, dst.sizeInBytes());
}

void clearImage(QImage &dst, const QRgb color) {
  visitSurface(dst, color, [](auto surface, auto color) {
    surface.overFill(color);
  });
}
