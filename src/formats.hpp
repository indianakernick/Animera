//
//  formats.hpp
//  Animera
//
//  Created by Indi Kernick on 25/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef formats_hpp
#define formats_hpp

#include "color.hpp"
#include <QtGui/qrgb.h>

// @TODO fixed order pixel formats
/*

union Union {
  Pixel pixel;
  uint8_t channels[4];
};

union Union {
  Pixel pixel;
  uint8_t channels[2];
};

*/

struct FormatARGB {
  using Pixel = QRgb;

  static constexpr int red(const Pixel pixel) {
    return qRed(pixel);
  }
  
  static constexpr int green(const Pixel pixel) {
    return qGreen(pixel);
  }
  
  static constexpr int blue(const Pixel pixel) {
    return qBlue(pixel);
  }
  
  static constexpr int alpha(const Pixel pixel) {
    return qAlpha(pixel);
  }

  static constexpr Color color(const Pixel pixel) {
    return {
      static_cast<uint8_t>(qRed(pixel)),
      static_cast<uint8_t>(qGreen(pixel)),
      static_cast<uint8_t>(qBlue(pixel)),
      static_cast<uint8_t>(qAlpha(pixel))
    };
  }
  
  static constexpr Pixel pixel(const int r, const int g, const int b, const int a = 255) {
    return pixel({
      static_cast<uint8_t>(r),
      static_cast<uint8_t>(g),
      static_cast<uint8_t>(b),
      static_cast<uint8_t>(a)
    });
  }
  
  static constexpr Pixel pixel(const Color color) {
    return qRgba(color.r, color.g, color.b, color.a);
  }
};

struct FormatIndex {
  const FormatARGB::Pixel *data;
  
  using Pixel = uint8_t;
  
  Color color(const uint8_t pixel) const {
    assert(data);
    return FormatARGB::color(data[pixel]);
  }
};

struct FormatYA {
  using Pixel = uint16_t;

  static constexpr int gray(const Pixel pixel) {
    return pixel & 0xFF;
  }
  static constexpr int alpha(const Pixel pixel) {
    return pixel >> 8;
  }
  static constexpr Pixel pixel(const int gray, const int alpha) {
    return (gray & 0xFF) | (alpha << 8);
  }

  static constexpr Color color(const Pixel pixel) {
    const uint8_t y = gray(pixel);
    const uint8_t a = alpha(pixel);
    return {y, y, y, a};
  }
  
  static constexpr Pixel pixel(const Color color) {
    return pixel(color.r, color.a);
  }
};

struct FormatY {
  using Pixel = uint8_t;
  
  static constexpr Color color(const Pixel pixel) {
    return {pixel, pixel, pixel, 255};
  }
  static constexpr Pixel pixel(const Color color) {
    return color.r;
  }
};

template <typename DstFormat, typename SrcFormat>
auto makeFormatConv(const DstFormat dstFmt, const SrcFormat srcFmt) {
  return [dstFmt, srcFmt](const typename SrcFormat::Pixel src) -> typename DstFormat::Pixel {
    return dstFmt.pixel(srcFmt.color(src));
  };
}

#endif
