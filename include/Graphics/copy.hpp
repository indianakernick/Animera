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

namespace gfx {

template <typename Pixel>
void copyRegion(
  const Surface<Pixel> dst,
  const CSurface<identity_t<Pixel>> src,
  const Point srcPos
) noexcept {
  const Rect srcRect = {srcPos, src.size()};
  const Rect dstRect = srcRect.intersected(dst.rect());
  if (dstRect.empty()) return;
  const size_t width = dstRect.s.w * sizeof(Pixel);
  auto srcRowIter = range(src, {dstRect.p - srcPos, dstRect.s}).begin();
  for (auto row : range(dst, dstRect)) {
    std::memcpy(row.begin(), srcRowIter.begin(), width);
    ++srcRowIter;
  }
}

template <typename Pixel>
void copy(
  const Surface<Pixel> dst,
  const CSurface<identity_t<Pixel>> src
) noexcept {
  assert(dst.size() == src.size());
  const size_t width = dst.width() * sizeof(Pixel);
  auto srcRowIter = range(src).begin();
  for (auto row : range(dst)) {
    std::memcpy(row.begin(), srcRowIter.begin(), width);
    ++srcRowIter;
  }
}

}

#endif
