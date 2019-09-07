//
//  copy.hpp
//  Animera
//
//  Created by Indi Kernick on 2/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_copy_hpp
#define graphics_copy_hpp

#include <cstring>
#include "traits.hpp"
#include "surface.hpp"
#include "iterator.hpp"

namespace gfx {

template <typename Pixel>
void copyRegion(
  const Surface<Pixel> dst,
  const CSurface<identity_t<Pixel>> src,
  const Point srcPos
) noexcept {
  const Rect srcRect = {srcPos, src.size()};
  const Rect dstRect = srcRect.intersected(dst.rect());
  if (!dstRect.empty()) {
    copy(dst.view(dstRect), src.view({{dstRect.p - srcPos}, dstRect.s}));
  }
}

template <typename Pixel>
void copy(
  const Surface<Pixel> dst,
  const CSurface<identity_t<Pixel>> src
) noexcept {
  assert(dst.size() == src.size());
  const size_t width = dst.width() * sizeof(Pixel);
  auto srcRowIter = begin(src);
  for (auto row : range(dst)) {
    std::memcpy(row.begin(), srcRowIter.begin(), width);
    ++srcRowIter;
  }
}

template <typename Pixel>
void overCopy(
  const Surface<Pixel> dst,
  const CSurface<identity_t<Pixel>> src
) noexcept {
  assert(dst.size() == src.size());
  assert(dst.pitch() == src.pitch());
  std::memcpy(dst.data(), src.data(), dst.pitch() * dst.height() * sizeof(Pixel));
}

}

#endif
