//
//  image.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "image.hpp"

#include "serial.hpp"

QTransform getTransform(const Image &img) {
  QTransform xform;
  xform.translate(img.xform.posX, img.xform.posY);
  xform.translate(img.data.width() / 2, img.data.height() / 2);
  xform.scale(flipToScale(img.xform.flipX), flipToScale(img.xform.flipY));
  xform.rotate(angleToDegrees(img.xform.angle));
  xform.translate(img.data.width() / -2, img.data.height() / -2);
  return xform;
}

QTransform getInvTransform(const Image &img) {
  return getTransform(img).inverted();
}

void serialize(QIODevice *dev, const Transform &xform) {
  assert(dev);
  serialize(dev, xform.posX);
  serialize(dev, xform.posY);
  serialize(dev, xform.angle);
  serialize(dev, xform.flipX);
  serialize(dev, xform.flipY);
}

void deserialize(QIODevice *dev, Transform &xform) {
  assert(dev);
  deserialize(dev, xform.posX);
  deserialize(dev, xform.posY);
  deserialize(dev, xform.angle);
  deserialize(dev, xform.flipX);
  deserialize(dev, xform.flipY);
}

void serialize(QIODevice *dev, const Image &image) {
  assert(dev);
  assert(!image.data.isNull());
  serialize(dev, image.xform);
  image.data.save(dev, "png");
}

void deserialize(QIODevice *dev, Image &image) {
  assert(dev);
  deserialize(dev, image.xform);
  image.data.load(dev, "png");
}

void serialize(QIODevice *dev, const Palette &palette) {
  assert(dev);
  serialize(dev, static_cast<uint16_t>(palette.size()));
  dev->write(
    reinterpret_cast<const char *>(palette.constData()),
    palette.size() * sizeof(Palette::value_type)
  );
}

void deserialize(QIODevice *dev, Palette &palette) {
  assert(dev);
  uint16_t size;
  deserialize(dev, size);
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

void copyImage(QImage &dst, const QImage &src) {
  assert(compatible(dst, src));
  dst.detach();
  std::memcpy(dst.bits(), src.constBits(), dst.sizeInBytes());
}

void clearImage(QImage &dst) {
  dst.detach();
  std::memset(dst.bits(), 0, dst.sizeInBytes());
}

namespace {

template <typename Pixel>
void clearImage(QImage &dst, const QRgb genericColor) {
  dst.detach();
  const Pixel color = static_cast<Pixel>(genericColor);
  const uintptr_t ppl = dst.bytesPerLine() / sizeof(Pixel);
  Pixel *row = reinterpret_cast<Pixel *>(dst.bits());
  Pixel *const lastRow = row + dst.height() * ppl;
  const uintptr_t width = dst.width();

  while (row != lastRow) {
    Pixel *pixel = row;
    Pixel *const lastPixel = row + width;
    while (pixel != lastPixel) {
      *pixel++ = color;
    }
    row += ppl;
  }
}

template <>
void clearImage<uint8_t>(QImage &dst, const QRgb genericColor) {
  dst.detach();
  std::memset(dst.bits(), static_cast<uint8_t>(genericColor), dst.sizeInBytes());
}

}

void clearImage(QImage &dst, const QRgb color) {
  if (dst.depth() == 32) {
    clearImage<uint32_t>(dst, color);
  } else if (dst.depth() == 8) {
    clearImage<uint8_t>(dst, color);
  } else {
    Q_UNREACHABLE();
  }
}
