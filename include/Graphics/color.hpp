//
//  color.hpp
//  Animera
//
//  Created by Indi Kernick on 1/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_color_hpp
#define graphics_color_hpp

#include <cstdint>

namespace gfx {

struct Color {
  uint8_t r, g, b, a;
};

constexpr uint8_t gray(const uint8_t r, const uint8_t g, const uint8_t b) noexcept {
  // https://poynton.ca/notes/colour_and_gamma/ColorFAQ.html#RTFToC9
  return (54 * r + 183 * g + 19 * b) / 256;
}

constexpr uint8_t gray(const Color color) noexcept {
  return gray(color.r, color.g, color.b);
}

}

#endif
