//
//  shrink.cpp
//  Graphics-Test
//
//  Created by Indi Kernick on 17/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "../catch.hpp"
#include <Graphics/draw.hpp>

namespace {

constexpr int size = 1024;
auto *srcData = new uint32_t[size * size];

}

TEST_CASE("shrink") {
  const gfx::Surface mutSrc{srcData, size, size, size};
  gfx::overFill(mutSrc);
  gfx::drawFilledCircle(mutSrc, 0x00FFEE00, {size / 2, size / 2}, size / 3);
  const gfx::Surface src = mutSrc.asConst();

  BENCHMARK("naive") {
    gfx::Point min = {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
    gfx::Point max = {std::numeric_limits<int>::min(), std::numeric_limits<int>::min()};
    int y = 0;
    for (auto row : gfx::range(src)) {
      int x = 0;
      for (auto pixel : row) {
        if (pixel) {
          min.x = std::min(min.x, x);
          min.y = std::min(min.y, y);
          max.x = std::max(max.x, x);
          max.y = std::max(max.y, y);
        }
        ++x;
      }
      ++y;
    } // 171,171  683, 683
    return gfx::Rect{min, (max - min + gfx::Point{1, 1}).size()};
  };
  
  BENCHMARK("fast") {
    auto allZeros = [](const uint32_t *begin, const uint32_t *end) noexcept {
      while (begin != end) {
        if (*begin != 0) return false;
        ++begin;
      }
      return true;
    };
    auto allZerosVert = [src](const uint32_t *begin, const uint32_t *end) noexcept {
      while (begin != end) {
        if (*begin != 0) return false;
        begin += src.pitch();
      }
      return true;
    };
  
    // there are off-by-one errors here
  
    const uint32_t *tlIter = src.ptr(0, 0);
    const uint32_t *trIter = src.ptr(src.width(), 0);
    const uint32_t *blIter = src.ptr(0, src.height());
    const uint32_t *brIter = src.ptr(src.width(), src.height());
  
    while (true) {
      if (tlIter == blIter) return gfx::Rect{};
      if (!allZeros(tlIter, trIter)) break;
      tlIter += src.pitch();
      trIter += src.pitch();
    }
    
    while (true) {
      if (tlIter == blIter) return gfx::Rect{};
      blIter -= src.pitch();
      brIter -= src.pitch();
      if (!allZeros(blIter, brIter)) break;
    }
    
    while (true) {
      if (tlIter == trIter) return gfx::Rect{};
      if (!allZerosVert(tlIter, blIter)) break;
      ++tlIter;
      ++blIter;
    }
    
    while (true) {
      if (tlIter == trIter) return gfx::Rect{};
      --trIter;
      --brIter;
      if (!allZerosVert(trIter, brIter)) break;
    }
    
    int x = static_cast<int>((tlIter - src.data()) % src.pitch());
    int y = static_cast<int>((tlIter - src.data()) / src.pitch());
    int width = static_cast<int>(trIter - tlIter) + 1;
    int height = static_cast<int>((blIter - tlIter) / src.pitch()) + 1;
    return gfx::Rect{{x, y}, {width, height}};
  };
}
