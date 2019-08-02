//
//  surface factory.hpp
//  Animera
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
  assert(!image.isNull());
  assert(image.depth() == sizeof(Pixel) * CHAR_BIT);
  // QImage::bits() is aligned to 4 bytes
  assert(image.bytesPerLine() % sizeof(Pixel) == 0);
  return {
    // non-const QImage::bits calls QImage::detach
    reinterpret_cast<Pixel *>(image.bits()),
    image.bytesPerLine() / static_cast<ptrdiff_t>(sizeof(Pixel)),
    image.width(),
    image.height()
  };
}

template <typename Pixel>
CSurface<Pixel> makeSurface(const QImage &image) {
  assert(!image.isNull());
  assert(image.depth() == sizeof(Pixel) * CHAR_BIT);
  // QImage::bits() is aligned to 4 bytes
  assert(image.bytesPerLine() % sizeof(Pixel) == 0);
  return {
    reinterpret_cast<const Pixel *>(image.bits()),
    image.bytesPerLine() / static_cast<ptrdiff_t>(sizeof(Pixel)),
    image.width(),
    image.height()
  };
}

template <typename Pixel>
CSurface<Pixel> makeCSurface(const QImage &image) {
  return makeSurface<const Pixel>(image);
}

template <typename Image, typename Func>
[[nodiscard]] decltype(auto) visitSurface(Image &image, Func &&func) {
  if (image.depth() == 32) {
    return func(makeSurface<uint32_t>(image));
  } else if (image.depth() == 8) {
    return func(makeSurface<uint8_t>(image));
  } else {
    Q_UNREACHABLE();
  }
}

template <typename Image, typename Func>
[[nodiscard]] decltype(auto) visitSurface(Image &image, const QRgb color, Func &&func) {
  return visitSurface(image, [color, &func](auto surface) {
    return func(surface, static_cast<typename decltype(surface)::pixel_type>(color));
  });
}

// If we ever need more than two, use a tuple
template <typename ImageA, typename ImageB, typename Func>
[[nodiscard]] decltype(auto) visitSurfaces(ImageA &a, ImageB &b, Func &&func) {
  assert(a.depth() == b.depth());
  if (a.depth() == 32) {
    return func(makeSurface<uint32_t>(a), makeSurface<uint32_t>(b));
  } else if (a.depth() == 8) {
    return func(makeSurface<uint8_t>(a), makeSurface<uint8_t>(b));
  } else {
    Q_UNREACHABLE();
  }
}

#endif
