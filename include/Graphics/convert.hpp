//
//  convert.hpp
//  Animera
//
//  Created by Indi Kernick on 8/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_convert_hpp
#define graphics_convert_hpp

#include "format.hpp"
#include "surface.hpp"
#include "iterator.hpp"

namespace gfx {

// @TODO allow Surface to deal with formats that don't fit in a single integer
template <typename Format, uint8_t Threshold>
Surface<uint8_t> convertToMono(const Surface<Pixel<Format>> src) noexcept {
  Surface<uint8_t> dst{
    src.template dataAs<uint8_t>(),
    src.template pitchAs<uint8_t>(),
    (src.width() + 7) / 8,
    src.height()
  };
  uint8_t *const dstRowEnd = dst.data() + dst.pitch() * dst.height();
  const ptrdiff_t width = src.width() / 8;
  const int extraBits = src.width() % 8;
  
  for (uint8_t *dstRowIter = dst.data(); dstRowIter != dstRowEnd; dstRowIter += dst.pitch()) {
    uint8_t *dstPixelIter = dstRowIter;
    const auto *srcPixelIter = reinterpret_cast<Pixel<Format> *>(dstPixelIter);
    uint8_t *const dstPixelEnd = dstPixelIter + width;
    
    const auto fillByte = [&srcPixelIter](const int bits) {
      uint8_t combinedPixel = 0;
      for (int i = 7; i > 7 - bits; --i) {
        const uint8_t gray = Format::color(*srcPixelIter++).r;
        combinedPixel |= (gray < Threshold ? 0 : 1) << i;
      }
      return combinedPixel;
    };
    
    while (dstPixelIter != dstPixelEnd) {
      *dstPixelIter++ = fillByte(8);
    }
    if (extraBits) {
      *dstPixelIter = fillByte(extraBits);
    }
  }
  
  return dst;
}

template <typename DstPixel, typename SrcPixel, typename DstFormat, typename SrcFormat>
Surface<DstPixel> convertInplace(
  Surface<SrcPixel> src,
  DstFormat dstFmt,
  SrcFormat srcFmt
) noexcept {
  static_assert(sizeof(DstPixel) <= sizeof(SrcPixel));
  
  Surface<DstPixel> dst{
    src.template dataAs<DstPixel>(),
    src.template pitchAs<DstPixel>(),
    src.width(),
    src.height()
  };
  
  for (auto dstRow : range(dst)) {
    const auto *srcPixelIter = reinterpret_cast<SrcPixel *>(dstRow.begin());
    for (DstPixel &dstPixel : dstRow) {
      dstPixel = dstFmt.pixel(srcFmt.color(*srcPixelIter++));
    }
  }
  
  return dst;
}

}

#endif
