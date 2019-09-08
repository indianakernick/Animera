//
//  porter duff.hpp
//  Animera
//
//  Created by Indi Kernick on 2/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_porter_duff_hpp
#define graphics_porter_duff_hpp

#include "each.hpp"
#include "format.hpp"

namespace gfx {

struct Factor {
  uint8_t a, b;
};

// Porter-Duff compositing with straight alpha
// https://keithp.com/~keithp/porterduff/p253-porter.pdf

// Using floats makes this a tiny bit faster
/*
Branchless is about 6 times slower!

inline Color porterDuff(const Factor f, const Color a, const Color b) {
  const uint32_t cA = a.a*f.a + b.a*f.b;
  const uint32_t flag = (static_cast<int32_t>(cA == 0) << 31) >> 31;
  const uint8_t cR = (a.a*f.a*a.r + b.a*f.b*b.r) / (cA | flag);
  const uint8_t cG = (a.a*f.a*a.g + b.a*f.b*b.g) / (cA | flag);
  const uint8_t cB = (a.a*f.a*a.b + b.a*f.b*b.b) / (cA | flag);
  return {cR, cG, cB, static_cast<uint8_t>(cA / 255)};
}
*/

inline Color porterDuff(const Factor f, const Color a, const Color b) {
  const uint32_t cA = a.a*f.a + b.a*f.b;
  if (cA == 0) {
    return {0, 0, 0, 0};
  } else {
    const uint8_t cR = (a.a*f.a*a.r + b.a*f.b*b.r) / cA;
    const uint8_t cG = (a.a*f.a*a.g + b.a*f.b*b.g) / cA;
    const uint8_t cB = (a.a*f.a*a.b + b.a*f.b*b.b) / cA;
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
Color porterDuff(const Mode mode, const Color a, const Color b) {
  return porterDuff(mode(a.a, b.a), a, b);
}

/*

struct Format {
  Format(const Format &);
  Color color(Pixel) const;
  Pixel pixel(Color) const;
};

*/

template <typename Mode, typename DstPixel, typename SrcPixel, typename DstFormat, typename SrcFormat>
void porterDuffRegion(
  const Mode mode,
  const Surface<DstPixel> dst,
  const CSurface<SrcPixel> src,
  const DstFormat dstFmt,
  const SrcFormat srcFmt,
  const Point srcPos
) {
  region(dst, src, srcPos, [mode, dstFmt, srcFmt](auto dstView, auto srcView) {
    porterDuff(mode, dstView, srcView, dstFmt, srcFmt);
  });
}

template <typename Mode, typename DstPixel, typename SrcPixel, typename DstFormat, typename SrcFormat>
void porterDuff(
  const Mode mode,
  const Surface<DstPixel> dst,
  const CSurface<SrcPixel> src,
  const DstFormat dstFmt,
  const SrcFormat srcFmt
) {
  each(dst, src, [mode, dstFmt, srcFmt](auto &dst, auto &src) {
    dst = dstFmt.pixel(porterDuff(
      mode,
      srcFmt.color(src),
      dstFmt.color(dst)
    ));
  });
}

}

#endif
