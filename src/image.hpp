//
//  image.hpp
//  Animera
//
//  Created by Indiana Kernick on 17/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_image_hpp
#define animera_image_hpp

#include <QtGui/qimage.h>
#include <Graphics/format.hpp>
#include <Graphics/pixel variant.hpp>

enum class Format : std::uint8_t {
  // cli new.cpp and init canvas dialog.cpp depend on the order
  rgba,
  index,
  gray
};

enum class PixelFormat {
  // cli export.cpp depends on order
  rgba,
  index,
  gray,
  gray_alpha,
  monochrome
};

constexpr QRgb mask_color_on = 0xFFFFFFFF;
constexpr QRgb mask_color_off = 0;

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
using RGBA = gfx::RGBA_LE;
using YA = gfx::YA_LE;
#else
using RGBA = gfx::RGBA_BE;
using YA = gfx::YA_BE;
#endif

// TODO: Use RGBA here
using FmtRgba = gfx::ARGB;
using FmtIndex = gfx::I<FmtRgba>;
using FmtGray = YA;

using PixelRgba = FmtRgba::Pixel;
using PixelIndex = FmtIndex::Pixel;
using PixelGray = FmtGray::Pixel;
using PixelMask = std::uint8_t;
using PixelVar = gfx::PixelVariant<PixelRgba, PixelIndex, PixelGray>;

template <typename Pixel>
constexpr QImage::Format qimageFormat();

template <>
constexpr QImage::Format qimageFormat<PixelRgba>() {
  return QImage::Format_ARGB32;
}

template <>
constexpr QImage::Format qimageFormat<PixelIndex>() {
  return QImage::Format_Grayscale8;
}

template <>
constexpr QImage::Format qimageFormat<PixelGray>() {
  return QImage::Format_Grayscale16; // Qt doesn't have a gray-alpha format
}

// template <>
// constexpr QImage::Format qimageFormat<PixelMask>() {
//   return QImage::Format_Grayscale8;
// }

constexpr QImage::Format qimageFormat(const Format format) {
  switch (format) {
    case Format::rgba:
      return qimageFormat<PixelRgba>();
    case Format::index:
      return qimageFormat<PixelIndex>();
    case Format::gray:
      return qimageFormat<PixelGray>();
    default:
      Q_UNREACHABLE();
  }
}

void copyImage(QImage &, const QImage &);
void clearImage(QImage &);
void clearImage(QImage &, QRect);

// TODO: Is a custom image worth considering?
// The non-const version is kind of annoying to use.
// I'm afraid of unnecessary copies happening in the background using either one
QImage view(QImage &, QRect);
QImage view(const QImage &, QRect);
QImage cview(const QImage &, QRect);

#endif
