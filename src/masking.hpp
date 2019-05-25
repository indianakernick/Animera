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
void maskClip(Surface<Pixel> dst, Surface<const uint8_t> msk) noexcept {
  assert(dst.size() == msk.size());
  auto mskRowIter = msk.range().begin();
  for (auto row : dst.range()) {
    const uint8_t *mskPixelIter = (*mskRowIter).begin();
    for (Pixel &pixel : row) {
      pixel &= spread<Pixel>(*mskPixelIter);
      ++mskPixelIter;
    }
    ++mskRowIter;
  }
}

/// Set pixels on the mask to the color while leaving the others unchanged
template <typename Pixel>
void maskFill(Surface<Pixel> dst, Surface<const uint8_t> msk, const Pixel color) noexcept {
  assert(dst.size() == msk.size());
  auto mskRowIter = msk.range().begin();
  for (auto row : dst.range()) {
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
void maskFillRegion(
  Surface<Pixel> dst,
  Surface<const uint8_t> msk,
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

/// Copy pixels on the mask while leaving the others unchanged
template <typename Pixel>
void maskCopy(Surface<Pixel> dst, Surface<const Pixel> src, Surface<const uint8_t> msk) noexcept {
  assert(dst.size() == src.size());
  assert(dst.size() == msk.size());
  auto srcRowIter = src.range().begin();
  auto mskRowIter = msk.range().begin();
  for (auto row : dst.range()) {
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

template <typename Pixel>
void maskCopyRegion(
  Surface<Pixel> dst,
  Surface<const Pixel> src,
  Surface<const uint8_t> msk,
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

template <typename Pixel>
void copyRegion(
  Surface<Pixel> dst,
  Surface<const Pixel> src,
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

#endif
