//
//  surface factory.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef surface_factory_hpp
#define surface_factory_hpp

#include "surface.hpp"
#include <QtGui/qimage.h>

// @TODO is there some way of dealing with the duplication?

template <typename Pixel>
Surface<Pixel> makeSurface(QImage &image) {
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
Surface<const Pixel> makeSurface(const QImage &image) {
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
