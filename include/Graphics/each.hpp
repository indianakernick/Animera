//
//  each.hpp
//  Animera
//
//  Created by Indi Kernick on 2/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_each_hpp
#define graphics_each_hpp

#include "surface.hpp"
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
  RowIterator bRowIter = range(b).begin();
  for (auto aRow : range(a)) {
    PixelB *bColIter = bRowIter.begin();
    for (PixelA &aPixel : aRow) {
      func(aPixel, *bColIter);
      ++bColIter;
    }
    ++bRowIter;
  }
}

// @TODO Does this belong somewhere else?
template <typename DstPixel, typename SrcPixel, typename Func>
void pixelTransform(Surface<DstPixel> dst, CSurface<SrcPixel> src, Func func) {
  each(dst, src, [func](DstPixel &dst, const SrcPixel &src) {
    dst = func(src);
  });
}

template <typename Pixel, typename Func>
void eachIntersect(Surface<Pixel> surface, const Rect rect, Func func) {
  for (auto row : range(surface, rect)) {
    for (Pixel &pixel : row) {
      func(pixel);
    }
  }
}

template <typename PixelA, typename PixelB, typename Func>
void eachIntersect(Surface<PixelA> a, Surface<PixelB> b, const Point bPos, Func func) {
  const Rect bRect = {bPos, b.size()};
  const Rect aRect = bRect.intersected(a.rect());
  if (aRect.empty()) return;
  RowIterator bRowIter = range(b, {aRect.p - bPos, aRect.s}).begin();
  for (auto aRow : range(a, aRect)) {
    PixelB *bColIter = bRowIter.begin();
    for (PixelA &aPixel : aRow) {
      func(aPixel, *bColIter);
      ++bColIter;
    }
    ++bRowIter;
  }
}

}

#endif
