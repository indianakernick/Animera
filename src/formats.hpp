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

  static constexpr int toRed(const Pixel pixel) {
    return qRed(pixel);
  }
  
  static constexpr int toGreen(const Pixel pixel) {
    return qGreen(pixel);
  }
  
  static constexpr int toBlue(const Pixel pixel) {
    return qBlue(pixel);
  }
  
  static constexpr int toAlpha(const Pixel pixel) {
    return qAlpha(pixel);
  }

  static constexpr Color toColor(const Pixel pixel) {
    return {
      static_cast<uint8_t>(qRed(pixel)),
      static_cast<uint8_t>(qGreen(pixel)),
      static_cast<uint8_t>(qBlue(pixel)),
      static_cast<uint8_t>(qAlpha(pixel))
    };
  }
  
  static constexpr Pixel toPixel(const Color color) {
    return qRgba(color.r, color.g, color.b, color.a);
  }
};

struct FormatIndex {
  const FormatARGB::Pixel *data;
  
  using Pixel = uint8_t;
  
  Color toColor(const uint8_t pixel) const {
    assert(data);
    return FormatARGB::toColor(data[pixel]);
  }
};

struct FormatYA {
  using Pixel = uint16_t;

  static constexpr int toGray(const Pixel pixel) {
    return pixel & 0xFF;
  }
  static constexpr int toAlpha(const Pixel pixel) {
    return pixel >> 8;
  }
  static constexpr Pixel toPixel(const int gray, const int alpha) {
    return (gray & 0xFF) | (alpha << 8);
  }

  static constexpr Color toColor(const Pixel pixel) {
    const uint8_t gray = toGray(pixel);
    const uint8_t alpha = toAlpha(pixel);
    return {gray, gray, gray, alpha};
  }
  
  static constexpr Pixel toPixel(const Color color) {
    return toPixel(color.r, color.a);
  }
};

struct FormatY {
  using Pixel = uint8_t;
  
  static constexpr Color toColor(const Pixel pixel) {
    return {pixel, pixel, pixel, 255};
  }
  static constexpr Pixel toPixel(const Color color) {
    return color.r;
  }
};

template <typename DstFormat, typename SrcFormat>
auto makeFormatConv(const DstFormat dstFmt, const SrcFormat srcFmt) {
  return [dstFmt, srcFmt](const typename SrcFormat::Pixel src) -> typename DstFormat::Pixel {
    return dstFmt.toPixel(srcFmt.toColor(src));
  };
}

#endif
