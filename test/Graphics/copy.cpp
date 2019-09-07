//
//  copy.cpp
//  Graphics-Test
//
//  Created by Indi Kernick on 7/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "../catch.hpp"

#include <Graphics/copy.hpp>

using Px = uint32_t;

namespace {

Px fn(const size_t x, const size_t y) {
  return static_cast<Px>((x + 1) * 3 + (y + 3) * 5);
}

}

TEST_CASE("copy") {
  constexpr size_t width = 6;
  constexpr size_t height = 4;
  
  Px dstArr[height][width];
  Px srcArr[height][width];
  std::memset(dstArr, 0, sizeof(dstArr));
  for (size_t y = 0; y != height; ++y) {
    for (size_t x = 0; x != width; ++x) {
      srcArr[y][x] = fn(x, y);
    }
  }
  
  SECTION("whole") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::Surface<Px> src{&srcArr[0][0], width, width, height};
    gfx::copy(dst, src);
    for (size_t y = 0; y != height; ++y) {
      for (size_t x = 0; x != width; ++x) {
        INFO("x: " << x);
        INFO("y: " << y);
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
}
