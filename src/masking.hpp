//
//  masking.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 8/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef masking_hpp
#define masking_hpp

#include "surface.hpp"

template <typename Uint>
Uint spread(const uint8_t byte) noexcept {
  static_assert(std::is_unsigned_v<Uint>);
  static_assert(std::is_integral_v<Uint>);
  constexpr size_t bits = CHAR_BIT * (sizeof(Uint) - 1);
  std::make_signed_t<Uint> integer = byte;
  integer <<= bits;
  integer >>= bits; // C++20 says this is a sign extension
  return integer;
}

/// Set pixels not on the mask to 0 while leaving the others unchanged
template <typename Pixel>
void maskClipRegion(
  const Surface<Pixel> dst,
  const CSurface<uint8_t> msk,
  const QPoint mskPos
) noexcept {
  const QRect srcRect = {mskPos, msk.size()};
  const QRect dstRect = srcRect.intersected(dst.rect());
  if (dstRect.isEmpty()) return;
  
  auto mskRowIter = msk.range({dstRect.topLeft() - mskPos, dstRect.size()}).begin();
  for (auto row : dst.range()) {
    const uint8_t *mskPixelIter = (*mskRowIter).begin();
    for (Pixel &pixel : row) {
      pixel &= spread<Pixel>(*mskPixelIter);
      ++mskPixelIter;
    }
    ++mskRowIter;
  }
}

/// Set pixels not on the mask to 0 while leaving the others unchanged
template <typename Pixel>
void maskClip(const Surface<Pixel> dst, const CSurface<uint8_t> msk) noexcept {
  Q_ASSUME(dst.size() == msk.size());
  maskClipRegion(dst, msk, {0, 0});
}

/// Set pixels on the mask to the color while leaving the others unchanged
template <typename Pixel>
void maskFillRegion(
  const Surface<Pixel> dst,
  const CSurface<uint8_t> msk,
  const Pixel color,
  const QPoint mskPos
) noexcept {
  const QRect mskRect = {mskPos, msk.size()};
  const QRect dstRect = mskRect.intersected(dst.rect());
  if (dstRect.isEmpty()) return;
  
  auto mskRowIter = msk.range({dstRect.topLeft() - mskPos, dstRect.size()}).begin();
  for (auto row : dst.range(dstRect)) {
    const uint8_t *mskPixelIter = (*mskRowIter).begin();
    for (Pixel &pixel : row) {
      const Pixel mskPx = spread<Pixel>(*mskPixelIter);
      pixel = (pixel & ~mskPx) | (color & mskPx);
      ++mskPixelIter;
    }
    ++mskRowIter;
  }
}

/// Set pixels on the mask to the color while leaving the others unchanged
template <typename Pixel>
void maskFill(
  const Surface<Pixel> dst,
  const CSurface<uint8_t> msk,
  const Pixel color
) noexcept {
  Q_ASSUME(dst.size() == msk.size());
  maskFillRegion(dst, msk, color, {0, 0});
}

/// Copy pixels on the mask while leaving the others unchanged
template <typename Pixel>
void maskCopyRegion(
  const Surface<Pixel> dst,
  const CSurface<Pixel> src,
  const CSurface<uint8_t> msk,
  const QPoint srcPos,
  const QPoint mskPos
) noexcept {
  const QRect srcRect = {srcPos, src.size()};
  const QRect mskRect = {mskPos, msk.size()};
  const QRect dstRect = srcRect.intersected(mskRect).intersected(dst.rect());
  if (dstRect.isEmpty()) return;
  
  auto srcRowIter = src.range({dstRect.topLeft() - srcPos, dstRect.size()}).begin();
  auto mskRowIter = msk.range({dstRect.topLeft() - mskPos, dstRect.size()}).begin();
  for (auto row : dst.range(dstRect)) {
    const Pixel *srcPixelIter = (*srcRowIter).begin();
    const uint8_t *mskPixelIter = (*mskRowIter).begin();
    for (Pixel &pixel : row) {
      const Pixel maskPx = spread<Pixel>(*mskPixelIter);
      pixel = (pixel & ~maskPx) | (*srcPixelIter & maskPx);
      ++srcPixelIter;
      ++mskPixelIter;
    }
    ++srcRowIter;
    ++mskRowIter;
  }
}

/// Copy pixels on the mask while leaving the others unchanged
template <typename Pixel>
void maskCopy(
  const Surface<Pixel> dst,
  const CSurface<Pixel> src,
  const CSurface<uint8_t> msk
) noexcept {
  Q_ASSUME(dst.size() == src.size());
  Q_ASSUME(dst.size() == msk.size());
  maskCopyRegion(dst, src, msk, {0, 0}, {0, 0});
}

/// Copy pixels
template <typename Pixel>
void copyRegion(
  const Surface<Pixel> dst,
  const CSurface<Pixel> src,
  const QPoint srcPos
) noexcept {
  const QRect srcRect = {srcPos, src.size()};
  const QRect dstRect = srcRect.intersected(dst.rect());
  if (dstRect.isEmpty()) return;
  
  auto srcRowIter = src.range({dstRect.topLeft() - srcPos, dstRect.size()}).begin();
  for (auto row : dst.range(dstRect)) {
    std::memcpy(row.begin(), (*srcRowIter).begin(), (row.end() - row.begin()) * sizeof(Pixel));
    ++srcRowIter;
  }
}

/// Copy pixels
template <typename Pixel>
void copy(
  const Surface<Pixel> dst,
  const CSurface<Pixel> src
) noexcept {
  Q_ASSUME(dst.size() == src.size());
  copyRegion(dst, src, {0, 0});
}

#endif
