//
//  porter duff.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 25/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef porter_duff_hpp
#define porter_duff_hpp

#include "surface.hpp"

struct Color {
  uint8_t r, g, b, a;
};

struct Factor {
  uint8_t a, b;
};

// Porter-Duff compositing with straight alpha
// https://keithp.com/~keithp/porterduff/p253-porter.pdf

inline Color porterDuff(const Color a, const Color b, const uint8_t aF, const uint8_t bF) {
  const uint32_t cA = a.a*aF + b.a*bF;
  if (cA == 0) {
    return {0, 0, 0, 0};
  } else {
    const uint8_t cR = (a.a*aF*a.r + b.a*bF*b.r) / cA;
    const uint8_t cG = (a.a*aF*a.g + b.a*bF*b.g) / cA;
    const uint8_t cB = (a.a*aF*a.b + b.a*bF*b.b) / cA;
    return {cR, cG, cB, static_cast<uint8_t>(cA / 255)};
  }
}

// Using a lambda to encourage inlining
#define MODE(NAME, FACTOR_A, FACTOR_B)                                          \
  constexpr auto mode_##NAME = [](                                              \
    [[maybe_unused]] const uint8_t a,                                           \
    [[maybe_unused]] const uint8_t b                                            \
  ) noexcept {                                                                  \
    return Factor{                                                              \
      static_cast<uint8_t>(FACTOR_A),                                           \
      static_cast<uint8_t>(FACTOR_B)                                            \
    };                                                                          \
  }

#define MODE_ALIAS(NAME, ALIAS, FACTOR_A, FACTOR_B)                             \
  MODE(NAME, FACTOR_A, FACTOR_B);                                               \
  constexpr auto mode_##ALIAS = mode_##NAME

MODE(      clear,              0,       0);
MODE_ALIAS(a,        src,      255,     0);
MODE_ALIAS(b,        dst,      0,       255);
MODE_ALIAS(a_over_b, src_over, 255,     255 - a);
MODE_ALIAS(b_over_a, dst_over, 255 - b, 255);
MODE_ALIAS(a_in_b,   src_in,   b,       0);
MODE_ALIAS(b_in_a,   dst_in,   0,       a);
MODE_ALIAS(a_out_b,  src_out,  255 - b, 0);
MODE_ALIAS(b_out_a,  dst_out,  0, 255 - a);
MODE_ALIAS(a_atop_b, src_atop, b, 255 - a);
MODE_ALIAS(b_atop_a, dst_atop, 255 - b, a);
MODE(      xor,                255 - b, 255 - a);

#undef MODE_ALIAS
#undef MODE

template <typename Mode>
Color porterDuff(const Color a, const Color b, const Mode mode) {
  const Factor factors = mode(a.a, b.a);
  return porterDuff(a, b, factors.a, factors.b);
}

/*

struct Format {
  static Color toColor(Pixel) const;
  static Pixel toPixel(Pixel) const;
};

*/

template <typename Format, typename Pixel, typename Mode>
void porterDuff(
  const Mode mode,
  Surface<Pixel> dst,
  Surface<const Pixel> src
) {
  assert(dst.size() == src.size());
  auto srcRowIter = src.range().begin();
  for (auto row : dst.range()) {
    const Pixel *srcPixelIter = (*srcRowIter).begin();
    for (Pixel &pixel : row) {
      pixel = Format::toPixel(porterDuff(
        Format::toColor(*srcPixelIter),
        Format::toColor(pixel),
        mode
      ));
      ++srcPixelIter;
    }
    ++srcRowIter;
  }
}

template <typename Format, typename Pixel, typename Mode>
void porterDuffRegion(
  const Mode mode,
  Surface<Pixel> dst,
  Surface<const Pixel> src,
  const QPoint srcPos
) {
  const QRect srcRect = {srcPos, src.size()};
  const QRect dstRect = srcRect.intersected(dst.rect());
  if (dstRect.isEmpty()) return;
  
  auto srcRowIter = src.range({dstRect.topLeft() - srcPos, dstRect.size()}).begin();
  for (auto row : dst.range(dstRect)) {
    const Pixel *srcPixelIter = (*srcRowIter).begin();
    for (Pixel &pixel : row) {
      pixel = Format::toPixel(porterDuff(
        Format::toColor(*srcPixelIter),
        Format::toColor(pixel),
        mode
      ));
      ++srcPixelIter;
    }
    ++srcRowIter;
  }
}

#endif
