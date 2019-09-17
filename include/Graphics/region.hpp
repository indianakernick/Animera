//
//  region.hpp
//  Animera
//
//  Created by Indi Kernick on 8/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_region_hpp
#define graphics_region_hpp

#include "surface.hpp"

namespace gfx {

template <typename Pixel, typename Func>
bool region(const Surface<Pixel> a, const Rect rect, Func func) {
  const Rect aRect = rect.intersected(a.rect());
  if (!aRect.empty()) {
    func(a.view(aRect));
    return true;
  }
  return false;
}

template <typename PixelA, typename PixelB, typename Func>
bool region(const Surface<PixelA> a, const Surface<PixelB> b, const Point bPos, Func func) {
  const Rect bRect = {bPos, b.size()};
  const Rect aRect = bRect.intersected(a.rect());
  if (!aRect.empty()) {
    func(a.view(aRect), b.view({{aRect.p - bPos}, aRect.s}));
    return true;
  }
  return false;
}

template <typename PixelA, typename PixelB, typename PixelC, typename Func>
bool region(
  const Surface<PixelA> a,
  const Surface<PixelB> b,
  const Surface<PixelC> c,
  const Point bPos,
  const Point cPos,
  Func func
) {
  const Rect bRect = {bPos, b.size()};
  const Rect cRect = {cPos, c.size()};
  const Rect aRect = bRect.intersected(cRect).intersected(a.rect());
  if (!aRect.empty()) {
    func(
      a.view(aRect),
      b.view(Rect{aRect.p - bPos, aRect.s}),
      c.view(Rect{aRect.p - cPos, aRect.s})
    );
    return true;
  }
  return false;
}

}

#endif
