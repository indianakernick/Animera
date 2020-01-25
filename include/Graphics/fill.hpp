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
#include "region.hpp"
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
  Pixel *firstPixel = dst.data() + dst.pitch() * first.y;
  Pixel *const lastPixel = firstPixel + last + 1;
  firstPixel += first.x;
  while (firstPixel != lastPixel) {
    *firstPixel++ = pixel;
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

// TODO: add fillRowRegion and fillColRegion to unit tests
// also test other Region functions
template <typename Pixel>
bool fillRowRegion(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  Point first,
  int last
) noexcept {
  if (!dst.containsY(first.y)) return false;
  first.x = first.x > 0 ? first.x : 0;
  last = last < dst.width() ? last : dst.width() - 1;
  if (first.x > last) return false;
  fillRow(dst, pixel, first, last);
  return true;
}

template <typename Pixel>
bool fillColRegion(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  Point first,
  int last
) noexcept {
  if (!dst.containsX(first.x)) return false;
  first.y = first.y > 0 ? first.y : 0;
  last = last < dst.height() ? last : dst.height() - 1;
  if (first.y > last) return false;
  fillCol(dst, pixel, first, last);
  return true;
}

template <typename Pixel>
bool fillRegion(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Rect rect
) noexcept {
  return region(dst, rect, [pixel](auto dstView) {
    fill(dstView, pixel);
  });
}

template <typename Pixel>
bool fillRegion(const Surface<Pixel> dst, const Rect rect) noexcept {
  return region(dst, rect, [](auto dstView) {
    fill(dstView);
  });
}

template <typename Pixel>
void fill(const Surface<Pixel> dst, const identity_t<Pixel> pixel) noexcept {
  for (auto dstRow : dst) {
    for (Pixel &dstPixel : dstRow) {
      dstPixel = pixel;
    }
  }
}

template <typename Pixel>
void fill(const Surface<Pixel> dst) noexcept {
  const size_t width = dst.byteWidth();
  for (auto row : dst) {
    std::memset(row.begin(), 0, width);
  }
}

template <typename Pixel>
void overFill(const Surface<Pixel> dst, const identity_t<Pixel> pixel) noexcept {
  Pixel *firstPixel = dst.data();
  Pixel *const lastPixel = firstPixel + dst.pitch() * dst.height();
  while (firstPixel != lastPixel) {
    *firstPixel++ = pixel;
  }
}

template <typename Pixel>
void overFill(const Surface<Pixel> dst) noexcept {
  std::memset(dst.data(), 0, dst.wholeByteSize());
}

}

#endif
