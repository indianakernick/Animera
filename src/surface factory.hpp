//
//  surface factory.hpp
//  Animera
//
//  Created by Indiana Kernick on 7/5/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_surface_factory_hpp
#define animera_surface_factory_hpp

#include <QtGui/qimage.h>
#include <Graphics/surface.hpp>

template <typename Pixel>
gfx::Surface<Pixel> makeSurface(QImage &image) {
  // QImage::bits is different so we can't call the const version
  assert(!image.isNull());
  assert(image.depth() == sizeof(Pixel) * CHAR_BIT);
  // QImage::bits() is aligned to 4 bytes
  assert(image.bytesPerLine() % sizeof(Pixel) == 0);
  return {
    reinterpret_cast<Pixel *>(image.bits()),
    image.bytesPerLine() / std::ptrdiff_t{sizeof(Pixel)},
    image.width(),
    image.height()
  };
}

template <typename Pixel>
gfx::CSurface<Pixel> makeSurface(const QImage &image) {
  assert(!image.isNull());
  assert(image.depth() == sizeof(Pixel) * CHAR_BIT);
  // QImage::bits() is aligned to 4 bytes
  assert(image.bytesPerLine() % sizeof(Pixel) == 0);
  return {
    reinterpret_cast<const Pixel *>(image.bits()),
    image.bytesPerLine() / std::ptrdiff_t{sizeof(Pixel)},
    image.width(),
    image.height()
  };
}

template <typename Pixel>
gfx::CSurface<Pixel> makeCSurface(const QImage &image) {
  return makeSurface<Pixel>(image);
}

namespace detail {

template <typename Pixel, typename Arg>
auto handleArg(Arg &&arg) noexcept {
  return makeSurface<Pixel>(std::forward<Arg>(arg));
}

template <typename Pixel, typename... Pixels>
auto handleArg(const gfx::PixelVariant<Pixels...> arg) noexcept {
  return static_cast<Pixel>(arg);
}

template <typename Tuple, std::size_t... Is>
[[nodiscard]] decltype(auto) visitSurfacesHelper(Tuple tuple, std::index_sequence<Is...>) {
  using Func = std::tuple_element_t<sizeof...(Is), Tuple>;
  switch (std::get<0>(tuple).depth()) {
    case 32: return std::forward<Func>(std::get<sizeof...(Is)>(tuple))(handleArg<PixelRgba> (std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
    case 16: return std::forward<Func>(std::get<sizeof...(Is)>(tuple))(handleArg<PixelGray> (std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
    case 8 : return std::forward<Func>(std::get<sizeof...(Is)>(tuple))(handleArg<PixelIndex>(std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
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

#endif
