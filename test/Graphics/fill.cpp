//
//  fill.cpp
//  Graphics-Test
//
//  Created by Indi Kernick on 8/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "../catch.hpp"
#include <Graphics/fill.hpp>

namespace {

constexpr uint32_t init_px32 = 0xFEDCBA98;
constexpr uint32_t fill_px32 = 0x76543210;

template <typename Element, size_t Rows, size_t Columns>
gfx::Surface<Element> makeSurface(Element (&array)[Rows][Columns]) noexcept {
  return {&array[0][0], Columns, Columns, Rows};
}

}

TEMPLATE_TEST_CASE("fillRow", "", uint8_t, uint16_t, uint32_t) {
  constexpr int width = 6;
  constexpr int height = 4;
  constexpr TestType init_px = TestType(init_px32);
  constexpr TestType fill_px = TestType(fill_px32);
  
  TestType dstArr[height][width];
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      dstArr[y][x] = init_px;
    }
  }
  
  SECTION("one") {
    const gfx::Point pos = {2, 3};
    gfx::fillRow(makeSurface(dstArr), fill_px, pos, pos.x);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (x == pos.x && y == pos.y) {
          REQUIRE(dstArr[y][x] == fill_px);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
  
  SECTION("middle") {
    const gfx::Point pos = {1, 2};
    const int last = 3;
    gfx::fillRow(makeSurface(dstArr), fill_px, pos, last);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (pos.x <= x && x <= last && y == pos.y) {
          REQUIRE(dstArr[y][x] == fill_px);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("fillCol", "", uint8_t, uint16_t, uint32_t) {
  constexpr int width = 5;
  constexpr int height = 9;
  constexpr TestType init_px = TestType(init_px32);
  constexpr TestType fill_px = TestType(fill_px32);
  
  TestType dstArr[height][width];
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      dstArr[y][x] = init_px;
    }
  }
  
  SECTION("one") {
    const gfx::Point pos = {3, 4};
    gfx::fillCol(makeSurface(dstArr), fill_px, pos, pos.y);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (x == pos.x && y == pos.y) {
          REQUIRE(dstArr[y][x] == fill_px);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
  
  SECTION("middle") {
    const gfx::Point pos = {2, 1};
    const int last = 5;
    gfx::fillCol(makeSurface(dstArr), fill_px, pos, last);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (pos.x == x && pos.y <= y && y <= last) {
          REQUIRE(dstArr[y][x] == fill_px);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("fillRegion", "", uint8_t, uint16_t, uint32_t) {
  constexpr int width = 10;
  constexpr int height = 10;
  constexpr TestType init_px = TestType(init_px32);
  constexpr TestType fill_px = TestType(fill_px32);
  
  TestType dstArr[height][width];
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      dstArr[y][x] = init_px;
    }
  }
  
  SECTION("no overlap pixel") {
    REQUIRE_FALSE(gfx::fillRegion(makeSurface(dstArr), fill_px, {{width, height}, {1, 1}}));
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == init_px);
      }
    }
  }
  
  SECTION("no overlap zero") {
    REQUIRE_FALSE(gfx::fillRegion(makeSurface(dstArr), {{width, height}, {1, 1}}));
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == init_px);
      }
    }
  }
  
  SECTION("top left pixel") {
    REQUIRE(gfx::fillRegion(makeSurface(dstArr), fill_px, {{-3, -4}, {4, 5}}));
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (x == 0 && y == 0) {
          REQUIRE(dstArr[y][x] == fill_px);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
  
  SECTION("top left zero") {
    REQUIRE(gfx::fillRegion(makeSurface(dstArr), {{-3, -4}, {4, 5}}));
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (x == 0 && y == 0) {
          REQUIRE(dstArr[y][x] == 0);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
  
  SECTION("middle pixel") {
    const gfx::Rect rect = {{3, 2}, {4, 5}};
    REQUIRE(gfx::fillRegion(makeSurface(dstArr), fill_px, rect));
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != height; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (rect.contains(gfx::Point{x, y})) {
          REQUIRE(dstArr[y][x] == fill_px);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
  
  SECTION("middle zero") {
    const gfx::Rect rect = {{3, 2}, {4, 5}};
    REQUIRE(gfx::fillRegion(makeSurface(dstArr), rect));
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != height; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (rect.contains(gfx::Point{x, y})) {
          REQUIRE(dstArr[y][x] == 0);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("fill", "", uint8_t, uint16_t, uint32_t) {
  constexpr int width = 7;
  constexpr int height = 11;
  constexpr TestType init_px = TestType(init_px32);
  constexpr TestType fill_px = TestType(fill_px32);
  
  TestType dstArr[height][width];
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      dstArr[y][x] = init_px;
    }
  }
  
  SECTION("whole pixel") {
    gfx::fill(makeSurface(dstArr), fill_px);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fill_px);
      }
    }
  }
  
  SECTION("whole zero") {
    gfx::fill(makeSurface(dstArr));
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == 0);
      }
    }
  }
  
  SECTION("middle pixel") {
    const gfx::Rect rect = {{2, 3}, {width - 4, height - 5}};
    gfx::fill(makeSurface(dstArr).view(rect), fill_px);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (rect.contains(gfx::Point{x, y})) {
          REQUIRE(dstArr[y][x] == fill_px);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
  
  SECTION("middle zero") {
    const gfx::Rect rect = {{2, 3}, {width - 4, height - 5}};
    gfx::fill(makeSurface(dstArr).view(rect));
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        if (rect.contains(gfx::Point{x, y})) {
          REQUIRE(dstArr[y][x] == 0);
        } else {
          REQUIRE(dstArr[y][x] == init_px);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("overFill", "", uint8_t, uint16_t, uint32_t) {
  constexpr int width = 6;
  constexpr int height = 3;
  constexpr TestType init_px = TestType(init_px32);
  constexpr TestType fill_px = TestType(fill_px32);
  
  TestType dstArr[height][width];
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x) {
      dstArr[y][x] = init_px;
    }
  }
  
  SECTION("pixel") {
    gfx::overFill(makeSurface(dstArr).view(gfx::Size{width - 2, height}), fill_px);
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == fill_px);
      }
    }
  }
  
  SECTION("zero") {
    gfx::overFill(makeSurface(dstArr).view(gfx::Size{width - 2, height}));
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        INFO("(" << x << ", " << y << ")");
        REQUIRE(dstArr[y][x] == 0);
      }
    }
  }
}
