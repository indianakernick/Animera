//
//  image.cpp
//  Animera
//
//  Created by Indiana Kernick on 17/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "image.hpp"

#include <Graphics/fill.hpp>
#include "surface factory.hpp"
#include "graphics convert.hpp"

void copyImage(QImage &dst, const QImage &src) {
  assert(dst.size() == src.size());
  assert(dst.format() == src.format());
  std::memcpy(dst.bits(), src.constBits(), dst.sizeInBytes());
}

void clearImage(QImage &dst) {
  std::memset(dst.bits(), 0, dst.sizeInBytes());
}

void clearImage(QImage &dst, const QRect rect) {
  visitSurface(dst, [rect](auto dst) {
    gfx::fillRegion(dst, convert(rect));
  });
}

namespace {

int index(const QImage &img, const QPoint pos) {
  return pos.y() * img.bytesPerLine() + pos.x() * img.depth() / 8;
}

}

QImage view(QImage &img, const QRect rect) {
  // can't call the const version because bits and the ctor are different
  if (img.isNull() || rect.isEmpty()) return {};
  assert(img.rect().contains(rect));
  return {
    img.bits() + index(img, rect.topLeft()),
    rect.width(),
    rect.height(),
    img.bytesPerLine(),
    img.format()
  };
}


QImage view(const QImage &img, const QRect rect) {
  if (img.isNull() || rect.isEmpty()) return {};
  assert(img.rect().contains(rect));
  return {
    img.bits() + index(img, rect.topLeft()),
    rect.width(),
    rect.height(),
    img.bytesPerLine(),
    img.format()
  };
}

QImage cview(const QImage &img, const QRect rect) {
  return view(img, rect);
}
