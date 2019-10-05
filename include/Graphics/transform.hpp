//
//  transform.hpp
//  Animera
//
//  Created by Indi Kernick on 6/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_transform_hpp
#define graphics_transform_hpp

#include "copy.hpp"
#include "each.hpp"

namespace gfx {

template <typename Pixel, typename Func>
void flipHori(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src, Func func) noexcept {
  assert(dst.size() == src.size());
  auto srcRowIter = begin(src);
  for (auto dstRow : range(dst)) {
    const Pixel *srcPixelIter = srcRowIter.end();
    for (Pixel &dstPixel : dstRow) {
      --srcPixelIter;
      func(dstPixel, *srcPixelIter);
    }
    ++srcRowIter;
  }
}

template <typename Pixel>
void flipHori(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  flipHori(dst, src, copyFunc<Pixel>);
}

template <typename Pixel, typename Func>
void flipVert(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src, Func func) noexcept {
  assert(dst.size() == src.size());
  auto srcRowIter = end(src);
  for (auto dstRow : range(dst)) {
    --srcRowIter;
    const Pixel *srcPixelIter = srcRowIter.begin();
    for (Pixel &dstPixel : dstRow) {
      func(dstPixel, *srcPixelIter);
      ++srcPixelIter;
    }
  }
}

template <typename Pixel>
void flipVert(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  assert(dst.size() == src.size());
  const size_t width = dst.width() * sizeof(Pixel);
  auto srcRowIter = end(src);
  for (auto dstRow : range(dst)) {
    --srcRowIter;
    std::memcpy(dstRow.begin(), srcRowIter.begin(), width);
  }
}

constexpr Size rotateSize(const Size srcSize, const int dir) noexcept {
  return (dir & 1) == 0 ? srcSize : srcSize.transposed();
}

template <typename Pixel, typename Func>
void rotate1(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src, Func func) noexcept {
  assert(dst.size() == src.size().transposed());
  const Pixel *srcColIter = src.data() + src.pitch() * src.height();
  for (auto dstRow : range(dst)) {
    const Pixel *srcRowIter = srcColIter;
    ++srcColIter;
    for (Pixel &dstPixel : dstRow) {
      srcRowIter -= src.pitch();
      func(dstPixel, *srcRowIter);
    }
  }
}

template <typename Pixel, typename Func>
void rotate2(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src, Func func) noexcept {
  assert(dst.size() == src.size());
  auto srcRowIter = end(src);
  for (auto dstRow : range(dst)) {
    --srcRowIter;
    const Pixel *srcPixelIter = srcRowIter.end();
    for (Pixel &dstPixel : dstRow) {
      --srcPixelIter;
      func(dstPixel, *srcPixelIter);
    }
  }
}

template <typename Pixel, typename Func>
void rotate3(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src, Func func) noexcept {
  assert(dst.size() == src.size().transposed());
  const Pixel *srcColIter = src.data() + src.width();
  for (auto dstRow : range(dst)) {
    --srcColIter;
    const Pixel *srcRowIter = srcColIter;
    for (Pixel &dstPixel : dstRow) {
      func(dstPixel, *srcRowIter);
      srcRowIter += src.pitch();
    }
  }
}

template <typename Pixel, typename Func>
void rotate(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src, const int dir, Func func) noexcept {
  assert(dst.size() == rotateSize(src.size(), dir));
  switch (dir & 3) {
    case 0: return each(dst, src, func);
    case 1: return rotate1(dst, src, func);
    case 2: return rotate2(dst, src, func);
    case 3: return rotate3(dst, src, func);
  }
}

template <typename Pixel>
void rotate1(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  rotate1(dst, src, copyFunc<Pixel>);
}

template <typename Pixel>
void rotate2(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  rotate2(dst, src, copyFunc<Pixel>);
}

template <typename Pixel>
void rotate3(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  rotate3(dst, src, copyFunc<Pixel>);
}

template <typename Pixel>
void rotate(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src, const int dir) noexcept {
  assert(dst.size() == rotateSize(src.size(), dir));
  switch (dir & 3) {
    case 0: return copy(dst, src);
    case 1: return rotate1(dst, src);
    case 2: return rotate2(dst, src);
    case 3: return rotate3(dst, src);
  }
}

template <typename Pixel, typename Func>
void scale(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src, Func func) noexcept {
  assert(dst.width() >= src.width());
  assert(dst.height() >= src.height());
  assert(dst.width() % src.width() == 0);
  assert(dst.height() % src.height() == 0);
  const Point factor = {dst.width() / src.width(), dst.height() / src.height()};
  auto iterate = [&](auto x1, auto y1) {
    Point skip = {};
    auto srcRowIter = begin(src);
    for (auto dstRow : range(dst)) {
      const Pixel *srcColIter = srcRowIter.begin();
      for (Pixel &dstPixel : dstRow) {
        func(dstPixel, *srcColIter);
        if (x1 || ++skip.x == factor.x) {
          skip.x = 0;
          ++srcColIter;
        }
      }
      if (y1 || ++skip.y == factor.y) {
        skip.y = 0;
        ++srcRowIter;
      }
    }
  };
  if (factor.x == 1) {
    iterate(std::bool_constant<true>{}, std::bool_constant<false>{});
  } else if (factor.y == 1) {
    iterate(std::bool_constant<false>{}, std::bool_constant<true>{});
  } else {
    iterate(std::bool_constant<false>{}, std::bool_constant<false>{});
  }
}

template <typename Pixel>
void scale(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  if (dst.size() == src.size()) {
    return copy(dst, src);
  }
  scale(dst, src, [](auto &dst, const auto src) {
    dst = src;
  });
}

template <typename Pixel, typename Func>
void spatialTransform(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src, Func func) noexcept {
  int y = 0;
  for (auto dstRow : range(dst)) {
    int x = 0;
    for (Pixel &dstPixel : dstRow) {
      dstPixel = src.ref(func(Point{x, y}));
      ++x;
    }
    ++y;
  }
}

inline Point flipHori(const Size dstSize, const Point dstPos) noexcept {
  return {dstSize.w - 1 - dstPos.x, dstPos.y};
}

inline Point flipVert(const Size dstSize, const Point dstPos) noexcept {
  return {dstPos.x, dstSize.h - 1 - dstPos.y};
}

inline Point rotate1(const Size dstSize, const Point dstPos) noexcept {
  return {dstPos.y, dstSize.w - 1 - dstPos.x};
}

inline Point rotate2(const Size dstSize, const Point dstPos) noexcept {
  return {dstSize.w - 1 - dstPos.x, dstSize.h - 1 - dstPos.y};
}

inline Point rotate3(const Size dstSize, const Point dstPos) noexcept {
  return {dstSize.h - 1 - dstPos.y, dstPos.x};
}

inline Point rotate(const int dir, const Size dstSize, const Point dstPos) noexcept {
  switch (dir & 3) {
    case 0: return dstPos;
    case 1: return rotate1(dstSize, dstPos);
    case 2: return rotate2(dstSize, dstPos);
    case 3: return rotate3(dstSize, dstPos);
    default: return {-1, -1}; // compiler doesn't realise this is impossible
  }
}

inline Point scale(const Point scale, const Point dstPos) noexcept {
  return {dstPos.x / scale.x, dstPos.y / scale.y};
}

}

#endif
