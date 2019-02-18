//
//  image.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "image.hpp"

#include "serial.hpp"
#include <QtGui/qpainter.h>

QImage Image::transformed() const {
  QImage newImage(data.size(), data.format());
  newImage.fill(0);
  QPainter painter(&newImage);
  painter.translate(xform.posX, xform.posY);
  painter.translate(data.width() / 2, data.height() / 2);
  painter.scale(flipToScale(xform.flipX), flipToScale(xform.flipY));
  painter.rotate(angleToDegrees(xform.angle));
  painter.translate(data.width() / -2, data.height() / -2);
  painter.drawImage(0, 0, data);
  painter.end();
  return newImage;
}

bool Image::isNull() const {
  return data.isNull();
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
