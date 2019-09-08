//
//  fill.hpp
//  Animera
//
//  Created by Indi Kernick on 2/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_fill_hpp
#define graphics_fill_hpp

#include <cstring>
#include "traits.hpp"
#include "iterator.hpp"

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
  assert(dst.contains(Point{last, first.y}));
  if constexpr (sizeof(Pixel) == 1) {
    std::memset(dst.ptr(first), pixel, last - first.x + 1);
  } else {
    Pixel *firstPixel = dst.data() + dst.pitch() * first.y;
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
  assert(dst.contains(Point{first.x, last}));
  Pixel *firstPixel = dst.data() + first.x;
  Pixel *const lastPixel = firstPixel + (last + 1) * dst.pitch();
  firstPixel += first.y * dst.pitch();
  while (firstPixel != lastPixel) {
    *firstPixel = pixel;
    firstPixel += dst.pitch();
  }
}

template <typename Pixel>
void fillRegion(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Rect rect
) noexcept {
  const Rect dstRect = rect.intersected(dst.rect());
  if (!dstRect.empty()) {
    fill(dst.view(dstRect), pixel);
  }
}

template <typename Pixel>
void fillRegion(const Surface<Pixel> dst, const Rect rect) noexcept {
  const Rect dstRect = rect.intersected(dst.rect());
  if (!dstRect.empty()) {
    fill(dst.view(dstRect));
  }
}

template <typename Pixel>
void fill(const Surface<Pixel> dst, const identity_t<Pixel> pixel) noexcept {
  for (auto dstRow : dst) {
    if constexpr (sizeof(Pixel) == 1) {
      std::memset(dstRow.begin(), pixel, dst.width());
    } else {
      for (Pixel &dstPixel : dstRow) {
        dstPixel = pixel;
      }
    }
  }
}

template <typename Pixel>
void fill(const Surface<Pixel> dst) noexcept {
  const size_t width = dst.width() * sizeof(Pixel);
  for (auto row : dst) {
    std::memset(row.begin(), 0, width);
  }
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
