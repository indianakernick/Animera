//
//  image.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "image.hpp"

#include "serial.hpp"
#include "surface factory.hpp"

void serialize(QIODevice *dev, const Image &image) {
  assert(dev);
  assert(!image.data.isNull());
  image.data.save(dev, "png");
}

void deserialize(QIODevice *dev, Image &image) {
  assert(dev);
  image.data.load(dev, "png");
}

void serialize(QIODevice *dev, const Palette &palette) {
  assert(dev);
  serializeBytes(dev, static_cast<uint16_t>(palette.size()));
  dev->write(
    reinterpret_cast<const char *>(palette.data()),
    palette.size() * sizeof(Palette::value_type)
  );
}

void deserialize(QIODevice *dev, Palette &palette) {
  assert(dev);
  uint16_t size;
  deserializeBytes(dev, size);
  palette.resize(size);
  dev->read(
    reinterpret_cast<char *>(palette.data()),
    palette.size() * sizeof(Palette::value_type)
  );
}

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
