//
//  common.hpp
//  Animera
//
//  Created by Indi Kernick on 27/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef common_hpp
#define common_hpp

#include "../catch.hpp"
#include <Graphics/surface.hpp>

using Px = uint16_t;

constexpr Px init = 0xFEDC;

constexpr Px fn(const int x, const int y) {
  return static_cast<Px>((x + 1) * 3 + (y + 3) * 5);
}

template <typename Element, size_t Rows, size_t Columns>
gfx::Surface<Element> makeSurface(Element (&array)[Rows][Columns]) {
  return {&array[0][0], Columns, Columns, Rows};
}

template <typename Element, size_t Rows, size_t Columns, typename Func>
void fill(Element (&array)[Rows][Columns], Func func) {
  for (int r = 0; r != Rows; ++r) {
    for (int c = 0; c != Columns; ++c) {
      array[r][c] = func(c, r);
    }
  }
}

template <typename Element, size_t Rows, size_t Columns>
void fill_fn(Element (&array)[Rows][Columns]) {
  fill(array, fn);
}

template <typename Element, size_t Rows, size_t Columns>
void fill_init(Element (&array)[Rows][Columns]) {
  fill(array, [](auto...) {
    return init;
  });
}

template <typename Element, size_t Rows, size_t Columns>
void fill_value(Element (&array)[Rows][Columns], const Element value) {
  fill(array, [value](auto...) {
    return value;
  });
}

#endif
