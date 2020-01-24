//
//  compare.hpp
//  Animera
//
//  Created by Indi Kernick on 24/1/20.
//  Copyright © 2020 Indi Kernick. All rights reserved.
//

#ifndef graphics_compare_hpp
#define graphics_compare_hpp

#include <cstring>
#include "region.hpp"
#include "traits.hpp"
#include "iterator.hpp"

namespace gfx {

template <typename Pixel>
bool equalRegion(
  const CSurface<Pixel> a,
  const CSurface<identity_t<Pixel>> b,
  const Point bPos
) noexcept {
  bool result = true;
  region(a, b, bPos, [&result](auto aView, auto bView) {
    result = equal(aView, bView);
  });
  return result;
}

template <typename Pixel>
bool equalRegion(
  const Surface<Pixel> a,
  const CSurface<identity_t<Pixel>> b,
  const Point bPos
) noexcept {
  return equalRegion(a.asConst(), b, bPos);
}

template <typename Pixel>
bool equal(
  const CSurface<Pixel> a,
  const CSurface<identity_t<Pixel>> b
) noexcept {
  if (a.size() != b.size()) return false;
  if (a.pitch() == b.pitch() && a.pitch() == a.width()) {
    return std::memcmp(a.data(), b.data(), a.byteSize()) == 0;
  } else {
    const size_t width = a.byteWidth();
    auto bRowIter = begin(b);
    for (auto aRow : range(a)) {
      if (std::memcmp(aRow.begin(), bRowIter.begin(), width) != 0) {
        return false;
      }
      ++bRowIter;
    }
    return true;
  }
}

template <typename Pixel>
bool equal(
  const Surface<Pixel> a,
  const CSurface<identity_t<Pixel>> b
) noexcept {
  return equal(a.asConst(), b);
}

template <typename Pixel>
bool equal(
  const CSurface<Pixel> src,
  const identity_t<Pixel> pixel
) noexcept {
  // TODO: pack small pixels into a uint64_t
  for (auto srcRow : range(src)) {
    for (const Pixel srcPixel : srcRow) {
      if (srcPixel != pixel) return false;
    }
  }
  return true;
}

template <typename Pixel>
bool equal(
  const Surface<Pixel> src,
  const identity_t<Pixel> pixel
) noexcept {
  return equal(src.asConst(), pixel);
}

}

#endif
