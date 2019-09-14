//
//  format.hpp
//  Animera
//
//  Created by Indi Kernick on 2/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_format_hpp
#define graphics_format_hpp

#include "color.hpp"

namespace gfx {

// @TODO fixed order pixel formats

struct FormatARGB {
  using Pixel = uint32_t;

  static constexpr uint8_t red(const Pixel pixel) noexcept {
    return (pixel >> 16) & 0xFF;
  }
  static constexpr uint8_t green(const Pixel pixel) noexcept {
    return (pixel >> 8) & 0xFF;
  }
  static constexpr uint8_t blue(const Pixel pixel) noexcept {
    return pixel & 0xFF;
  }
  static constexpr uint8_t alpha(const Pixel pixel) noexcept {
    return pixel >> 24;
  }
  
  static constexpr Pixel pixel(const int r, const int g, const int b, const int a = 255) noexcept {
    return pixel({
      static_cast<uint8_t>(r),
      static_cast<uint8_t>(g),
      static_cast<uint8_t>(b),
      static_cast<uint8_t>(a)
    });
  }

  static constexpr Color color(const Pixel pixel) noexcept {
    return {red(pixel), green(pixel), blue(pixel), alpha(pixel)};
  }
  static constexpr Pixel pixel(const Color color) noexcept {
    return (Pixel{color.a} << 24) |
           (Pixel{color.r} << 16) |
           (Pixel{color.g} <<  8) |
            Pixel{color.b};
  }
};

template <typename Target = FormatARGB>
struct FormatI {
  const typename Target::Pixel *data;
  
  using Pixel = uint8_t;
  
  Color color(const Pixel pixel) const noexcept {
    assert(data);
    return Target::color(data[pixel]);
  }
};

struct FormatYA {
  using Pixel = uint16_t;

  static constexpr uint8_t gray(const Pixel pixel) noexcept {
    return pixel & 255;
  }
  static constexpr uint8_t alpha(const Pixel pixel) noexcept {
    return pixel >> 8;
  }
  static constexpr Pixel pixel(const int gray, const int alpha) noexcept {
    return pixel({
      static_cast<uint8_t>(gray), 0, 0, static_cast<uint8_t>(alpha)
    });
  }

  static constexpr Color color(const Pixel pixel) noexcept {
    return {gray(pixel), gray(pixel), gray(pixel), alpha(pixel)};
  }
  
  static constexpr Pixel pixel(const Color color) noexcept {
    return (color.a << 8) | color.r;
  }
};

struct FormatY {
  using Pixel = uint8_t;
  
  static constexpr Color color(const Pixel pixel) noexcept {
    return {pixel, pixel, pixel, 255};
  }
  static constexpr Pixel pixel(const Color color) noexcept {
    return color.r;
  }
};

}

#endif
