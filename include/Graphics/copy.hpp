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
#include "region.hpp"
#include "traits.hpp"
#include "iterator.hpp"

namespace gfx {

template <typename Pixel>
bool copyRegion(
  const Surface<Pixel> dst,
  const CSurface<identity_t<Pixel>> src,
  const Point srcPos
) noexcept {
  return region(dst, src, srcPos, [](auto dstView, auto srcView) {
    copy(dstView, srcView);
  });
}

template <typename Pixel>
void copy(
  const Surface<Pixel> dst,
  const CSurface<identity_t<Pixel>> src
) noexcept {
  assert(dst.size() == src.size());
  const size_t width = dst.byteWidth();
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
  std::memcpy(dst.data(), src.data(), dst.wholeByteSize());
}

}

#endif
