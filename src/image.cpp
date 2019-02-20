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
