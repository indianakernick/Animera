//
//  visit.hpp
//  Animera
//
//  Created by Indi Kernick on 8/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_visit_hpp
#define graphics_visit_hpp

#include <tuple>
#include <utility>
#include "image.hpp"
#include "surface.hpp"

namespace gfx {

template <typename Pixel>
Surface<Pixel> makeSurface(Image &image) noexcept {
  assert(image.depth() == sizeof(Pixel));
  return {
    reinterpret_cast<Pixel *>(image.data()),
    image.pitch() / ptrdiff_t{sizeof(Pixel)},
    image.width(),
    image.height()
  };
}

template <typename Pixel>
CSurface<Pixel> makeSurface(const Image &image) noexcept {
  assert(image.depth() == sizeof(Pixel));
  return {
    reinterpret_cast<const Pixel *>(image.data()),
    image.pitch() / ptrdiff_t{sizeof(Pixel)},
    image.width(),
    image.height()
  };
}

template <typename Pixel>
CSurface<Pixel> makeCSurface(const Image &image) noexcept {
  return makeSurface<Pixel>(image);
}

namespace detail {

template <typename Pixel, typename Arg>
auto handleArg(Arg &&arg) noexcept {
  if constexpr (std::is_integral_v<std::decay_t<Arg>>) {
    return static_cast<Pixel>(arg);
  } else {
    return makeSurface<Pixel>(std::forward<Arg>(arg));
  }
}

template <typename Tuple, size_t... Is>
[[nodiscard]] decltype(auto) visitSurfacesHelper(Tuple tuple, std::index_sequence<Is...>) {
  using Func = std::tuple_element_t<sizeof...(Is), Tuple>;
  switch (std::get<0>(tuple).depth()) {
    case 8: return std::forward<Func>(std::get<sizeof...(Is)>(tuple))(handleArg<uint64_t>(std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
    case 4: return std::forward<Func>(std::get<sizeof...(Is)>(tuple))(handleArg<uint32_t>(std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
    case 2: return std::forward<Func>(std::get<sizeof...(Is)>(tuple))(handleArg<uint16_t>(std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
    case 1: return std::forward<Func>(std::get<sizeof...(Is)>(tuple))(handleArg<uint8_t> (std::forward<std::tuple_element_t<Is, Tuple>>(std::get<Is>(tuple)))...);
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
  return visitSurfaces(image, std::forward<Func>(func));
}

}

#endif
