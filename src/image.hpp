//
//  image.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef image_hpp
#define image_hpp

#include <QtGui/qimage.h>
#include <QtCore/qvector.h>
#include <QtGui/qtransform.h>

using Palette = QVector<QRgb>;

struct Transform {
  qint16 posX = 0;
  qint16 posY = 0;
  quint8 angle = 0;
  bool flipX = false;
  bool flipY = false;
};

enum class Format : uint8_t {
  color,
  palette
};

struct Image {
  QImage data;
  Transform xform;
};

constexpr QImage::Format color_format = QImage::Format_ARGB32;
constexpr QImage::Format palette_format = QImage::Format_Grayscale8;

inline QImage::Format getImageFormat(const Format format) {
  return format == Format::color ? color_format : palette_format;
}

inline quint8 increaseAngle(const quint8 angle) {
  return (angle + 1) & 3;
}

inline quint8 decreaseAngle(const quint8 angle) {
  return (angle + 3) & 3;
}

inline qreal angleToDegrees(const quint8 angle) {
  return angle * 90.0;
}

inline qreal flipToScale(const bool flip) {
  return static_cast<qreal>(flip) * -2.0 + 1.0;
}

QTransform getTransform(const Image &);
QTransform getInvTransform(const Image &);

void serialize(QIODevice *, const Transform &);
void deserialize(QIODevice *, Transform &);

void serialize(QIODevice *, const Image &);
void deserialize(QIODevice *, Image &);

void serialize(QIODevice *, const Palette &);
void deserialize(QIODevice *, Palette &);

#endif
