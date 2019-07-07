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

enum class Format : uint8_t {
  color,
  palette,
  gray
};

constexpr QImage::Format mask_format = QImage::Format_Grayscale8;
constexpr QRgb mask_color_on = 0xFFFFFFFF;
constexpr QRgb mask_color_off = 0;

constexpr QImage::Format qimageFormat(const Format format) {
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

using PixelColor = QRgb;
using PixelPalette = uint8_t;
using PixelGray = uint8_t;
using PixelMask = uint8_t;

bool compatible(const QImage &, const QImage &);
QImage makeCompatible(const QImage &);
QImage makeMask(QSize);
void copyImage(QImage &, const QImage &);
void clearImage(QImage &);
void clearImage(QImage &, QRgb);

#endif
