//
//  formats.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 25/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef formats_hpp
#define formats_hpp

#include "color.hpp"
#include <QtGui/qrgb.h>

struct FormatARGB {
  Color toColor(const QRgb pixel) const {
    return {
      static_cast<uint8_t>(qRed(pixel)),
      static_cast<uint8_t>(qGreen(pixel)),
      static_cast<uint8_t>(qBlue(pixel)),
      static_cast<uint8_t>(qAlpha(pixel))
    };
  }
  
  QRgb toPixel(const Color color) const {
    return qRgba(color.r, color.g, color.b, color.a);
  }
};

struct FormatPalette {
  const QRgb *data;
  
  Color toColor(const uint8_t pixel) const {
    assert(data);
    return FormatARGB{}.toColor(data[pixel]);
  }
};

struct FormatGray {
  Color toColor(const uint8_t pixel) const {
    return {pixel, pixel, pixel, 255};
  }
};

#endif
