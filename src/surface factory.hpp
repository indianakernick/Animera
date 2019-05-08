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
  assert(image.depth() == sizeof(Pixel) * CHAR_BIT);
  // QImage::bits() is aligned to 4 bytes
  assert(image.bytesPerLine() % sizeof(Pixel) == 0);
  assert(!image.isNull());
  image.detach();
  return {
    reinterpret_cast<Pixel *>(image.bits()),
    image.bytesPerLine() / static_cast<ptrdiff_t>(sizeof(Pixel)),
    image.width(),
    image.height()
  };
}

template <typename Pixel>
Surface<const Pixel> makeSurface(const QImage &image) {
  // image doesn't need to be detached
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

template <typename Func>
[[nodiscard]] decltype(auto) makeSurface(QImage &image, Func &&func) {
  if (image.depth() == 32) {
    return func(makeSurface<uint32_t>(image));
  } else if (image.depth() == 8) {
    return func(makeSurface<uint8_t>(image));
  } else {
    Q_UNREACHABLE();
  }
}

template <typename Func>
 [[nodiscard]] decltype(auto) makeSurface(const QImage &image, Func &&func) {
  return makeSurface(const_cast<QImage &>(image), [&func](auto surface) {
    return func(surface.addConst());
  });
}

template <typename Func>
[[nodiscard]] decltype(auto) makeSurface(QImage &image, const QRgb color, Func &&func) {
  return makeSurface(image, [color, &func](auto surface) {
    return func(surface, static_cast<typename decltype(surface)::pixel_type>(color));
  });
}

template <typename Func>
[[nodiscard]] decltype(auto) makeSurface(const QImage &image, const QRgb color, Func &&func) {
  return makeSurface(const_cast<QImage &>(image), color, [&func](auto surface, auto color) {
    return func(surface.addConst(), color);
  });
}

#endif
