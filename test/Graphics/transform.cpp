//
//  transform.cpp
//  Graphics-Test
//
//  Created by Indi Kernick on 13/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "common.hpp"
#include <Graphics/transform.hpp>

TEST_CASE("transform") {
  constexpr int width = 7;
  constexpr int height = 5;
  
  Px dstArr[height][width];
  Px srcArr[height][width];
  fill_init(dstArr);
  fill_fn(srcArr);
  
  SECTION("pixelTransform") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::pureEach(dst, src, [](Px px) {
      return px + 42;
    });
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y) + 42);
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("flipHori") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::flipHori(dst, src);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(width - 1 - x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("flipVert") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::flipVert(dst, src);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, height - 1 - y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("rotate1") {
    gfx::Surface<Px> dst{&dstArr[0][0], height, height, width};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::rotate1(dst, src);
    for (int y = 0; y != width; ++y) {
      for (int x = 0; x != height; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dst.ref(x, y) == fn(y, height - 1 - x));
      }
    }
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("rotate2") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::rotate2(dst, src);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(width - 1 - x, height - 1 - y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("rotate3") {
    gfx::Surface<Px> dst{&dstArr[0][0], height, height, width};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::rotate3(dst, src);
    for (int y = 0; y != width; ++y) {
      for (int x = 0; x != height; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dst.ref(x, y) == fn(width - 1 - y, x));
      }
    }
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("rotate(8) = copy") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::rotate(dst, src, 8);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(x, y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("rotate(-7) = rotate1") {
    gfx::Surface<Px> dst{&dstArr[0][0], height, height, width};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::rotate(dst, src, -7);
    for (int y = 0; y != width; ++y) {
      for (int x = 0; x != height; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dst.ref(x, y) == fn(y, height - 1 - x));
      }
    }
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("rotate(6) = rotate2") {
    gfx::Surface<Px> dst{&dstArr[0][0], width, width, height};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::rotate(dst, src, 6);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fn(width - 1 - x, height - 1 - y));
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
  
  SECTION("rotate(-5) = rotate3") {
    gfx::Surface<Px> dst{&dstArr[0][0], height, height, width};
    gfx::CSurface<Px> src{&srcArr[0][0], width, width, height};
    gfx::rotate(dst, src, -5);
    for (int y = 0; y != width; ++y) {
      for (int x = 0; x != height; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dst.ref(x, y) == fn(width - 1 - y, x));
      }
    }
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(srcArr[y][x] == fn(x, y));
      }
    }
  }
}

TEMPLATE_TEST_CASE_SIG("scale", "", ((int X, int Y), X, Y),
  (1, 1), (1, 3), (3, 1), (3, 3), (3, 2), (2, 3)
) {
  constexpr int width = 3;
  constexpr int height = 4;
  
  Px srcArr[height][width];
  fill_fn(srcArr);
  Px dstArr[height * Y][width * X];
  fill_init(dstArr);
  gfx::scale(makeSurface(dstArr), makeSurface(srcArr));
  for (int y = 0; y != height * Y; ++y) {
    for (int x = 0; x != width * X; ++x) {
      INFO("(" << x << ", " << y << ")");
      REQUIRE(dstArr[y][x] == fn(x / X, y / Y));
    }
  }
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      INFO("(" << x << ", " << y << ")");
      REQUIRE(srcArr[y][x] == fn(x, y));
    }
  }
}
