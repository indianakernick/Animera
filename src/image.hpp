//
//  image.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef image_hpp
#define image_hpp

#include <vector>
#include <QtGui/qimage.h>
#include <QtGui/qtransform.h>

using Palette = std::vector<QRgb>;

struct Transform {
  qint16 posX = 0;
  qint16 posY = 0;
  quint8 angle = 0;
  bool flipX = false;
  bool flipY = false;
};

enum class Format : uint8_t {
  color,
  palette,
  gray
};

struct Image {
  QImage data;
  Transform xform;
  Palette *palette = nullptr;
};

constexpr QImage::Format mask_format = QImage::Format_Grayscale8;
constexpr QRgb mask_color_on = 0xFFFFFFFF;
constexpr QRgb mask_color_off = 0;

constexpr QImage::Format getImageFormat(const Format format) {
  switch (format) {
    case Format::color:
      return QImage::Format_ARGB32;
    case Format::palette:
      return QImage::Format_Grayscale8;
    case Format::gray:
      return QImage::Format_Grayscale8;
    default:
      Q_UNREACHABLE();
  }
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

bool compatible(const QImage &, const QImage &);
QImage makeCompatible(const QImage &);
QImage makeMask(QSize);
void copyImage(QImage &, const QImage &);
void clearImage(QImage &);
void clearImage(QImage &, QRgb);

#endif
