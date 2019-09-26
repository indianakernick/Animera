//
//  flood fill.hpp
//  Animera
//
//  Created by Indi Kernick on 3/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_flood_fill_hpp
#define graphics_flood_fill_hpp

#include "geometry.hpp"

namespace gfx {

/*

struct Policy {
  // start with this pixel. Returns whether we should continue
  bool start(Point);
  // size of the underlying surface
  Size size() const;
  // should this pixel be filled?
  bool check(Point) const;
  // fill a pixel
  void set(Point);
};

*/

namespace detail {

inline Point up(const Point p) {
  return {p.x, p.y - 1};
}

inline Point right(const Point p) {
  return {p.x + 1, p.y};
}

inline Point down(const Point p) {
  return {p.x, p.y + 1};
}

inline Point left(const Point p) {
  return {p.x - 1, p.y};
}

inline int min(const int a, const int b) {
  return a < b ? a : b;
}

inline int max(const int a, const int b) {
  return a < b ? b : a;
}

struct Bounds {
  Point min;
  Point max;
};

// Flood Fill algorithm by Adam Milazzo
// http://www.adammil.net/blog/v126_A_More_Efficient_Flood_Fill.html

template <typename Policy>
void floodFillStart(Policy &&, Point, Size, Bounds &);

template <typename Policy>
void floodFillCore(Policy &&policy, Point pos, const Size size, Bounds &bounds) {
  bounds.min.x = min(bounds.min.x, pos.x);
  bounds.min.y = min(bounds.min.y, pos.y);
  int lastRowLength = 0;
  do {
    int rowLength = 0;
    Point start = pos;
    if (lastRowLength != 0 && !policy.check(pos)) {
      do {
        if (--lastRowLength == 0) return;
        pos = right(pos);
      } while (!policy.check(pos));
    } else {
      while (pos.x != 0 && policy.check(left(pos))) {
        pos = left(pos);
        policy.set(pos);
        if (pos.y != 0 && policy.check(up(pos))) {
          floodFillStart(policy, up(pos), size, bounds);
        }
        ++rowLength;
        ++lastRowLength;
      }
      bounds.min.x = min(bounds.min.x, pos.x);
    }
    bool movedRight = false;
    while (start.x < size.w && policy.check(start)) {
      movedRight = true;
      policy.set(start);
      start = right(start);
      ++rowLength;
    }
    if (movedRight) {
      bounds.max.x = max(bounds.max.x, start.x - 1);
      bounds.max.y = max(bounds.max.y, start.y);
    }
    if (rowLength < lastRowLength) {
      const int endX = pos.x + lastRowLength;
      while (++start.x < endX) {
        if (policy.check(start)) {
          floodFillCore(policy, start, size, bounds);
        }
      }
    } else if (rowLength > lastRowLength && pos.y != 0) {
      Point above = up({pos.x + lastRowLength, pos.y});
      while (++above.x < start.x) {
        if (policy.check(above)) {
          floodFillStart(policy, above, size, bounds);
        }
      }
    }
    lastRowLength = rowLength;
    pos = down(pos);
  } while (lastRowLength != 0 && pos.y < size.h);
}

template <typename Policy>
void floodFillStart(Policy &&policy, Point pos, const Size size, Bounds &bounds) {
  while (true) {
    const Point startPos = pos;
    while (pos.y != 0 && policy.check(up(pos))) pos = up(pos);
    while (pos.x != 0 && policy.check(left(pos))) pos = left(pos);
    if (pos == startPos) break;
  }
  floodFillCore(policy, pos, size, bounds);
}

}

template <typename Policy>
Rect floodFill(Policy &&policy, const Point pos) {
  if (!policy.start(pos)) return {};
  detail::Bounds bounds = {pos, pos};
  detail::floodFillStart(policy, pos, policy.size(), bounds);
  return {bounds.min, (bounds.max - bounds.min + Point{1, 1}).size()};
}

}

#endif
