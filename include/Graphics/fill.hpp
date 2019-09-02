//
//  fill.hpp
//  Animera
//
//  Created by Indi Kernick on 2/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_fill_hpp
#define graphics_fill_hpp

#include <cassert>
#include <cstring>
#include "traits.hpp"
#include "surface.hpp"

namespace gfx {

template <typename Pixel>
void fillRow(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point first,
  const int last
) noexcept {
  assert(first.x <= last);
  assert(dst.contains(first));
  assert(dst.contains({last, first.y}));
  if constexpr (sizeof(Pixel) == 1) {
    std::memset(dst.ptr(first), pixel, last - first.x + 1);
  } else {
    Pixel *firstPixel = dst.pitch() * first.y;
    Pixel *const lastPixel = firstPixel + last + 1;
    firstPixel += first.x;
    while (firstPixel != lastPixel) {
      *firstPixel++ = pixel;
    }
  }
}

template <typename Pixel>
void fillCol(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point first,
  const int last
) noexcept {
  assert(first.y <= last);
  assert(dst.contains(first));
  assert(dst.contains({first.x, last}));
  Pixel *firstPixel = first.x;
  Pixel *const lastPixel = firstPixel + (last + 1) * dst.pitch();
  firstPixel += first.y * dst.pitch();
  while (firstPixel != lastPixel) {
    *firstPixel = pixel;
    firstPixel += dst.pitch();
  }
}

template <typename Pixel>
void fillRect(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Rect rect
) noexcept {
  assert(!rect.empty());
  assert(dst.contains(rect));
  Pixel *firstRow = dst.ptr(rect.p);
  Pixel *const lastRow = firstRow + dst.pitch() * rect.s.h;
  while (firstRow != lastRow) {
    if constexpr (sizeof(Pixel) == 1) {
      std::memset(firstRow, pixel, dst.width());
    } else {
      Pixel *firstPixel = firstRow;
      Pixel *const lastPixel = firstPixel + dst.width();
      while (firstPixel != lastPixel) {
        *firstPixel++ = pixel;
      }
    }
    firstRow += dst.pitch();
  }
}

template <typename Pixel>
void fillRect(const Surface<Pixel> dst, const Rect rect) noexcept {
  assert(!rect.empty());
  assert(dst.contains(rect));
  Pixel *firstRow = dst.ptr(rect.p);
  Pixel *const lastRow = firstRow + dst.pitch() * rect.s.h;
  while (firstRow != lastRow) {
    std::memset(firstRow, 0, dst.width() * sizeof(Pixel));
    firstRow += dst.pitch();
  }
}

template <typename Pixel>
void fill(const Surface<Pixel> dst, const identity_t<Pixel> pixel) noexcept {
  fillRect(dst, pixel, dst.rect());
}

template <typename Pixel>
void fill(const Surface<Pixel> dst) noexcept {
  fillRect(dst, dst.rect());
}

template <typename Pixel>
void overFill(const Surface<Pixel> dst, const identity_t<Pixel> pixel) noexcept {
  if constexpr (sizeof(Pixel) == 1) {
    std::memset(dst.data(), pixel, dst.pitch() * dst.height());
  } else {
    Pixel *firstPixel = dst.data();
    Pixel *const lastPixel = firstPixel + dst.pitch() * dst.height();
    while (firstPixel != lastPixel) {
      *firstPixel++ = pixel;
    }
  }
}

template <typename Pixel>
void overFill(const Surface<Pixel> dst) noexcept {
  std::memset(dst.data(), 0, dst.pitch() * dst.height() * sizeof(Pixel));
}

}

#endif
