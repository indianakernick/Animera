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
  constexpr size_t width = 4;
  constexpr size_t height = 3;
  constexpr Px init = 0xFEDC;
  
  Px dstArr[height][width];
  Px srcArr[height][width];
  for (size_t y = 0; y != height; ++y) {
    for (size_t x = 0; x != width; ++x) {
      dstArr[y][x] = init;
    }
  }
  for (size_t y = 0; y != height; ++y) {
    for (size_t x = 0; x != width; ++x) {
      srcArr[y][x] = fn(x, y);
    }
  }
  
  SECTION("whole") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::copy(dst, src);
    for (size_t y = 0; y != height; ++y) {
      for (size_t x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("miss right column") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width - 1, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width - 1, height};
    gfx::copy(dst, src);
    for (size_t y = 0; y != height; ++y) {
      for (size_t x = 0; x != width - 1; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
      INFO("y: " << y);
      REQUIRE(dstArr[y][width - 1] == init);
      REQUIRE(srcArr[y][width - 1] == fn(width - 1, y));
    }
  }
  
  SECTION("miss left column") {
    gfx::Surface<Px> dst{&dstArr[0][1], width, width - 1, height};
    gfx::CSurface<Px> src{&srcArr[0][1], width, width - 1, height};
    gfx::copy(dst, src);
    for (size_t y = 0; y != height; ++y) {
      for (size_t x = 1; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
      INFO("y: " << y);
      REQUIRE(dstArr[y][0] == init);
      REQUIRE(srcArr[y][0] == fn(0, y));
    }
  }
  
  SECTION("miss bottom row") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height - 1};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height - 1};
    gfx::copy(dst, src);
    for (size_t y = 0; y != height - 1; ++y) {
      for (size_t x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
    for (size_t x = 0; x != width; ++x) {
      INFO("x: " << x);
      REQUIRE(dstArr[height - 1][x] == init);
      REQUIRE(srcArr[height - 1][x] == fn(x, height - 1));
    }
  }
  
  SECTION("miss top row") {
    gfx::Surface<Px> dst{&dstArr[1][0], width, width, height - 1};
    gfx::CSurface<Px> src{&srcArr[1][0], width, width, height - 1};
    gfx::copy(dst, src);
    for (size_t y = 1; y != height; ++y) {
      for (size_t x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
    for (size_t x = 0; x != width; ++x) {
      INFO("x: " << x);
      REQUIRE(dstArr[0][x] == init);
      REQUIRE(srcArr[0][x] == fn(x, 0));
    }
  }
}
