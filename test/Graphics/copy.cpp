//
//  copy.cpp
//  Graphics-Test
//
//  Created by Indi Kernick on 7/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "../catch.hpp"
#include <Graphics/copy.hpp>

namespace {

using Px = uint16_t;

constexpr Px init = 0xFEDC;

Px fn(const int x, const int y) {
  return static_cast<Px>((x + 1) * 3 + (y + 3) * 5);
}

}

TEST_CASE("copyRegion") {
  constexpr int width = 10;
  constexpr int height = 7;
  
  Px dstArr[height][width];
  Px srcArr[height][width];
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      dstArr[y][x] = init;
    }
  }
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      srcArr[y][x] = fn(x, y);
    }
  }
  
  SECTION("no overlap") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::copyRegion(dst, src, {width, height});
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == init);
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("top left pixel") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    const gfx::Point srcPos = {1 - width, 1 - height};
    gfx::copyRegion(dst, src, srcPos);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (x == 0 && y == 0) {
          REQUIRE(dstArr[y][x] == fn(x - srcPos.x, y - srcPos.y));
          REQUIRE(srcArr[y][x] == fn(x, y));
        } else {
          REQUIRE(dstArr[y][x] == init);
          REQUIRE(srcArr[y][x] == fn(x, y));
        }
      }
    }
  }
  
  SECTION("middle") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    const gfx::Rect srcRect = {{2, 3}, {width - 4, height - 5}};
    gfx::CSurface<Px> src{&srcArr[0][0], width, srcRect.s};
    gfx::copyRegion(dst, src, srcRect.p);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (srcRect.contains({x, y})) {
          REQUIRE(dstArr[y][x] == fn(x - srcRect.p.x, y - srcRect.p.y));
          REQUIRE(srcArr[y][x] == fn(x, y));
        } else {
          REQUIRE(dstArr[y][x] == init);
          REQUIRE(srcArr[y][x] == fn(x, y));
        }
      }
    }
  }
}

TEST_CASE("copy") {
  constexpr int width = 3;
  constexpr int height = 4;
  
  Px dstArr[height][width];
  Px srcArr[height][width];
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      dstArr[y][x] = init;
    }
  }
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      srcArr[y][x] = fn(x, y);
    }
  }
  
  SECTION("whole") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::copy(dst, src);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
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
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width - 1; ++x) {
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
    for (int y = 0; y != height; ++y) {
      for (int x = 1; x != width; ++x) {
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
    for (int y = 0; y != height - 1; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
    for (int x = 0; x != width; ++x) {
      INFO("x: " << x);
      REQUIRE(dstArr[height - 1][x] == init);
      REQUIRE(srcArr[height - 1][x] == fn(x, height - 1));
    }
  }
  
  SECTION("miss top row") {
    gfx::Surface<Px> dst{&dstArr[1][0], width, width, height - 1};
    gfx::CSurface<Px> src{&srcArr[1][0], width, width, height - 1};
    gfx::copy(dst, src);
    for (int y = 1; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
    for (int x = 0; x != width; ++x) {
      INFO("x: " << x);
      REQUIRE(dstArr[0][x] == init);
      REQUIRE(srcArr[0][x] == fn(x, 0));
    }
  }
}

TEST_CASE("overCopy") {
  constexpr int width = 5;
  constexpr int height = 5;
  
  Px dstArr[height][width];
  Px srcArr[height][width];
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      dstArr[y][x] = init;
    }
  }
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      srcArr[y][x] = fn(x, y);
    }
  }
  
  SECTION("whole") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width - 2, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width - 2, height};
    gfx::overCopy(dst, src);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
}
