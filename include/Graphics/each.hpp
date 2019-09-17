//
//  each.hpp
//  Animera
//
//  Created by Indi Kernick on 2/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_each_hpp
#define graphics_each_hpp

#include "region.hpp"
#include "iterator.hpp"

namespace gfx {

template <typename Pixel, typename Func>
void each(Surface<Pixel> surface, Func func) {
  for (auto row : range(surface)) {
    for (Pixel &pixel : row) {
      func(pixel);
    }
  }
}

template <typename PixelA, typename PixelB, typename Func>
void each(Surface<PixelA> a, Surface<PixelB> b, Func func) {
  assert(a.size() == b.size());
  auto bRowIter = begin(b);
  for (auto aRow : range(a)) {
    PixelB *bColIter = bRowIter.begin();
    for (PixelA &aPixel : aRow) {
      func(aPixel, *bColIter);
      ++bColIter;
    }
    ++bRowIter;
  }
}

template <typename PixelA, typename PixelB, typename PixelC, typename Func>
void each(Surface<PixelA> a, Surface<PixelB> b, Surface<PixelC> c, Func func) {
  assert(a.size() == b.size());
  assert(b.size() == c.size());
  auto bRowIter = begin(b);
  auto cRowIter = begin(c);
  for (auto aRow : range(a)) {
    PixelB *bColIter = bRowIter.begin();
    PixelC *cColIter = cRowIter.begin();
    for (PixelA &aPixel : aRow) {
      func(aPixel, *bColIter, *cColIter);
      ++bColIter;
      ++cColIter;
    }
    ++bRowIter;
    ++cRowIter;
  }
}

template <typename Pixel, typename Func>
bool eachRegion(Surface<Pixel> surface, const Rect rect, Func func) {
  return region(surface, rect, [func](auto view) {
    each(view, func);
  });
}

template <typename PixelA, typename PixelB, typename Func>
bool eachRegion(Surface<PixelA> a, Surface<PixelB> b, const Point bPos, Func func) {
  return region(a, b, bPos, [func](auto aView, auto bView) {
    each(aView, bView, func);
  });
}

template <typename PixelA, typename PixelB, typename PixelC, typename Func>
bool eachRegion(
  Surface<PixelA> a,
  Surface<PixelB> b,
  Surface<PixelC> c,
  const Point bPos,
  const Point cPos,
  Func func
) {
  return region(a, b, c, bPos, cPos, [func](auto aView, auto bView, auto cView) {
    each(aView, bView, cView, func);
  });
}

}

#endif
