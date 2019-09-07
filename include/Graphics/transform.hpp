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

/*

// Given a pixel on the src image, return a pixel on the dst image
DstPixel xform(SrcPixel)

*/

template <typename DstPixel, typename SrcPixel, typename XForm>
void pixelTransform(Surface<DstPixel> dst, CSurface<SrcPixel> src, XForm xform) {
  each(dst, src, [xform](DstPixel &dst, const SrcPixel &src) {
    dst = xform(src);
  });
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

template <typename Pixel>
void flipHori(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  assert(dst.size() == src.size());
  auto srcRowIter = begin(src);
  for (auto dstRow : range(dst)) {
    const Pixel *srcPixelIter = srcRowIter.end();
    for (Pixel &dstPixel : dstRow) {
      --srcPixelIter;
      dstPixel = *srcPixelIter;
    }
    ++srcRowIter;
  }
}

constexpr Size rotateSize(const Size srcSize, const int dir) noexcept {
  return (dir & 1) == 0 ? srcSize : srcSize.transposed();
}

template <typename Pixel>
void rotate1(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  assert(dst.size() == src.size().transposed());
  const Pixel *srcColIter = src.data() + src.pitch() * src.height();
  for (auto dstRow : range(dst)) {
    const Pixel *srcRowIter = srcColIter;
    ++srcColIter;
    for (Pixel &dstPixel : dstRow) {
      srcRowIter -= src.pitch();
      dstPixel = *srcRowIter;
    }
  }
}

template <typename Pixel>
void rotate2(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  assert(dst.size() == src.size());
  auto srcRowIter = end(src);
  for (auto dstRow : range(dst)) {
    --srcRowIter;
    const Pixel *srcPixelIter = srcRowIter.end();
    for (Pixel &dstPixel : dstRow) {
      --srcPixelIter;
      dstPixel = *srcPixelIter;
    }
  }
}

template <typename Pixel>
void rotate3(Surface<Pixel> dst, CSurface<identity_t<Pixel>> src) noexcept {
  assert(dst.size() == src.size().transposed());
  const Pixel *srcColIter = src.data() + src.width();
  for (auto dstRow : range(dst)) {
    --srcColIter;
    const Pixel *srcRowIter = srcColIter;
    for (Pixel &dstPixel : dstRow) {
      dstPixel = *srcRowIter;
      srcRowIter += src.pitch();
    }
  }
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

}

#endif
