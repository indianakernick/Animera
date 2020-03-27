//
//  image.hpp
//  Animera
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef image_hpp
#define image_hpp

#include <QtGui/qimage.h>

enum class Format : std::uint8_t {
  // cli.cpp depends on the order
  rgba,
  index,
  gray
};

constexpr QImage::Format mask_format = QImage::Format_Grayscale8;
constexpr QRgb mask_color_on = 0xFFFFFFFF;
constexpr QRgb mask_color_off = 0;

constexpr QImage::Format qimageFormat(const Format format) {
  switch (format) {
    case Format::rgba:
      return QImage::Format_ARGB32;
    case Format::index:
      return QImage::Format_Grayscale8;
    case Format::gray:
      return QImage::Format_Grayscale16; // Qt doesn't have a gray-alpha format
    default:
      Q_UNREACHABLE();
  }
}

template <typename Pixel>
constexpr QImage::Format qimageFormat();

template <>
constexpr QImage::Format qimageFormat<std::uint8_t>() {
  return QImage::Format_Grayscale8;
}

template <>
constexpr QImage::Format qimageFormat<std::uint16_t>() {
  return QImage::Format_Grayscale16;
}

template <>
constexpr QImage::Format qimageFormat<std::uint32_t>() {
  return QImage::Format_ARGB32;
}

using PixelRgba = QRgb;
using PixelIndex = std::uint8_t;
using PixelGray = std::uint16_t;
using PixelMask = std::uint8_t;

bool compatible(const QImage &, const QImage &);
QImage makeCompatible(const QImage &);
QImage makeMask(QSize);
void copyImage(QImage &, const QImage &);
void clearImage(QImage &);
void clearImage(QImage &, QRgb);
void clearImage(QImage &, QRect);

// TODO: Is a custom image worth considering?
// The non-const version is kind of annoying to use.
// I'm afraid of unnecessary copies happening in the background using either one
QImage view(QImage &, QRect);
QImage view(const QImage &, QRect);
QImage cview(const QImage &, QRect);

#endif
