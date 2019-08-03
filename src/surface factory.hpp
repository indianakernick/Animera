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

namespace detail {

template <typename Tuple, size_t... Is>
[[nodiscard]] decltype(auto) visitSurfacesHelper(Tuple tuple, std::index_sequence<Is...>) {
  using Func = std::tuple_element_t<sizeof...(Is), Tuple>;
  Func func = std::get<sizeof...(Is)>(tuple);
  switch (std::get<0>(tuple).depth()) {
    case 32: return std::forward<Func>(func)(makeSurface<uint32_t>(std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
    case 16: return std::forward<Func>(func)(makeSurface<uint16_t>(std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
    case 8 : return std::forward<Func>(func)(makeSurface<uint8_t> (std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
    default: Q_UNREACHABLE();
  }
}

}

template <typename... Args>
[[nodiscard]] decltype(auto) visitSurfaces(Args &&... args) {
  static_assert(sizeof...(Args) > 1);
  return detail::visitSurfacesHelper(
    std::forward_as_tuple(args...),
    std::make_index_sequence<sizeof...(Args) - 1>{}
  );
}

template <typename Image, typename Func>
[[nodiscard]] decltype(auto) visitSurface(Image &image, Func &&func) {
  return visitSurfaces(image, func);
}

template <typename Image, typename Func>
[[nodiscard]] decltype(auto) visitSurface(Image &image, const QRgb color, Func &&func) {
  return visitSurface(image, [color, &func](auto surface) {
    return func(surface, static_cast<typename decltype(surface)::pixel_type>(color));
  });
}

#endif
