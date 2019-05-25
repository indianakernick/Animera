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

// @TODO Need a PaletteFormat that stores the palette

struct ARGB_Format {
  static Color toColor(const QRgb pixel) {
    return {
      static_cast<uint8_t>(qRed(pixel)),
      static_cast<uint8_t>(qGreen(pixel)),
      static_cast<uint8_t>(qBlue(pixel)),
      static_cast<uint8_t>(qAlpha(pixel))
    };
  }
  static QRgb toPixel(const Color color) {
    return qRgba(color.r, color.g, color.b, color.a);
  }
};

#endif
