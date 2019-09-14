//
//  mask.hpp
//  Animera
//
//  Created by Indi Kernick on 8/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_mask_hpp
#define graphics_mask_hpp

#include "each.hpp"
#include "traits.hpp"
#include "region.hpp"
#include "surface.hpp"

namespace gfx {

constexpr uint8_t mask_off = 0;
constexpr uint8_t mask_on = 0xFF;

namespace detail {

template <typename Uint>
Uint spread(const uint8_t byte) noexcept {
  static_assert(std::is_unsigned_v<Uint>);
  static_assert(std::is_integral_v<Uint>);
  constexpr size_t bits = 8 * (sizeof(Uint) - 1);
  std::make_signed_t<Uint> integer = byte;
  integer <<= bits;
  integer >>= bits; // C++20 says this is a sign extension
  return integer;
}

}

template <typename Pixel>
void maskClip(const Surface<Pixel> dst, const CSurface<uint8_t> msk) noexcept {
  each(dst, msk, [](Pixel &dstPx, const uint8_t mskPx) {
    dstPx &= detail::spread<Pixel>(mskPx);
  });
}

template <typename Pixel>
void maskClipRegion(
  const Surface<Pixel> dst,
  const CSurface<uint8_t> msk,
  const Point mskPos
) noexcept {
  region(dst, msk, mskPos, [](auto dstView, auto mskView) {
    maskClip(dstView, mskView);
  });
}

template <typename Pixel>
void maskFill(
  const Surface<Pixel> dst,
  const CSurface<uint8_t> msk,
  const identity_t<Pixel> pixel
) noexcept {
  each(dst, msk, [pixel](Pixel &dstPx, const uint8_t mskPx) {
    const Pixel mask = detail::spread<Pixel>(mskPx);
    dstPx = (dstPx & ~mask) | (pixel & mask);
  });
}

template <typename Pixel>
void maskFillRegion(
  const Surface<Pixel> dst,
  const CSurface<uint8_t> msk,
  const Pixel pixel,
  const Point mskPos
) noexcept {
  region(dst, msk, mskPos, [pixel](auto dstView, auto mskView) {
    maskFill(dstView, mskView, pixel);
  });
}

template <typename Pixel>
void maskCopy(
  const Surface<Pixel> dst,
  const CSurface<identity_t<Pixel>> src,
  const CSurface<uint8_t> msk
) noexcept {
  each(dst, src, msk, [](Pixel &dstPx, const Pixel srcPx, const uint8_t mskPx) {
    const Pixel mask = detail::spread<Pixel>(mskPx);
    dstPx = (dstPx & ~mask) | (srcPx & mask);
  });
}

template <typename Pixel>
void maskCopyRegion(
  const Surface<Pixel> dst,
  const CSurface<identity_t<Pixel>> src,
  const CSurface<uint8_t> msk,
  const Point srcPos,
  const Point mskPos
) noexcept {
  region(dst, src, msk, srcPos, mskPos, [](auto dstView, auto srcView, auto mskView) {
    maskCopy(dstView, srcView, mskView);
  });
}

}

#endif
