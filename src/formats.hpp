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

struct FormatARGB {
  using Pixel = QRgb;

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

struct FormatPalette {
  const QRgb *data;
  
  using Pixel = uint8_t;
  
  Color toColor(const uint8_t pixel) const {
    assert(data);
    return FormatARGB{}.toColor(data[pixel]);
  }
};

struct FormatGray {
  using Pixel = uint16_t;

  static constexpr int toGray(const uint16_t pixel) {
    return pixel & 0xFF;
  }
  static constexpr int toAlpha(const uint16_t pixel) {
    return pixel >> 8;
  }
  static constexpr uint16_t toPixel(const int gray, const int alpha) {
    return (gray & 0xFF) | (alpha << 8);
  }

  static constexpr Color toColor(const uint16_t pixel) {
    const uint8_t gray = toGray(pixel);
    const uint8_t alpha = toAlpha(pixel);
    return {gray, gray, gray, alpha};
  }
  
  static constexpr uint16_t toPixel(const Color color) {
    return toPixel(color.r, color.a);
  }
};

#endif
