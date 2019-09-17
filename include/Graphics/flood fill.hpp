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

// Flood Fill algorithm by Adam Milazzo
// http://www.adammil.net/blog/v126_A_More_Efficient_Flood_Fill.html

template <typename Policy>
void floodFillStart(Policy &&, Point, Size);

template <typename Policy>
void floodFillCore(Policy &&policy, Point pos, const Size size) {
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
          floodFillStart(policy, up(pos), size);
        }
        ++rowLength;
        ++lastRowLength;
      }
    }
    while (start.x < size.w && policy.check(start)) {
      policy.set(start);
      start = right(start);
      ++rowLength;
    }
    if (rowLength < lastRowLength) {
      const int endX = pos.x + lastRowLength;
      while (++start.x < endX) {
        if (policy.check(start)) {
          floodFillCore(policy, start, size);
        }
      }
    } else if (rowLength > lastRowLength && pos.y != 0) {
      Point above = up({pos.x + lastRowLength, pos.y});
      while (++above.x < start.x) {
        if (policy.check(above)) {
          floodFillStart(policy, above, size);
        }
      }
    }
    lastRowLength = rowLength;
    pos = down(pos);
  } while (lastRowLength != 0 && pos.y < size.h);
}

template <typename Policy>
void floodFillStart(Policy &&policy, Point pos, const Size size) {
  while (true) {
    const Point startPos = pos;
    while (pos.y != 0 && policy.check(up(pos))) pos = up(pos);
    while (pos.x != 0 && policy.check(left(pos))) pos = left(pos);
    if (pos == startPos) break;
  }
  floodFillCore(policy, pos, size);
}

}

// @TODO return the filled bounds

template <typename Policy>
bool floodFill(Policy &&policy, const Point pos) {
  if (!policy.start(pos)) return false;
  detail::floodFillStart(policy, pos, policy.size());
  return true;
}

}

#endif
