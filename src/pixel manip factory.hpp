//
//  pixel manip factory.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef pixel_manip_factory_hpp
#define pixel_manip_factory_hpp

#include <QtGui/qimage.h>
#include "pixel manip.hpp"

template <typename Pixel>
PixelManip<Pixel> makePixelManip(QImage &image) {
  assert(image.isDetached());
  assert(image.depth() == sizeof(Pixel) * CHAR_BIT);
  // QImage::bits() is aligned to 4 bytes
  assert(image.bytesPerLine() % sizeof(Pixel) == 0);
  assert(!image.isNull());
  return {
    reinterpret_cast<Pixel *>(image.bits()),
    image.bytesPerLine() / static_cast<ptrdiff_t>(sizeof(Pixel)),
    image.width(),
    image.height()
  };
}

template <typename Pixel>
PixelManip<const Pixel> makePixelManip(const QImage &image) {
  assert(image.isDetached());
  assert(image.depth() == sizeof(Pixel) * CHAR_BIT);
  // QImage::bits() is aligned to 4 bytes
  assert(image.bytesPerLine() % sizeof(Pixel) == 0);
  assert(!image.isNull());
  return {
    reinterpret_cast<const Pixel *>(image.bits()),
    image.bytesPerLine() / static_cast<ptrdiff_t>(sizeof(Pixel)),
    image.width(),
    image.height()
  };
}

#endif
