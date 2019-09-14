//
//  draw.hpp
//  Animera
//
//  Created by Indi Kernick on 8/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_draw_hpp
#define graphics_draw_hpp

#include "fill.hpp"
#include "surface.hpp"
#include "flood fill.hpp"

namespace gfx {

template <typename Pixel>
void drawFilledRect(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Rect rect
) noexcept {
  assert(dst.contains(rect));
  fill(dst.view(rect), pixel);
}

template <typename Pixel>
void drawStrokedRect(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Rect outer,
  const Rect inner
) noexcept {
  assert(dst.contains(outer));
  if (inner.empty()) {
    return fill(dst.view(outer), pixel);
  }
  assert(outer.contains(inner));
  const Rect sides[4] = {
    {outer.p, {outer.s.w, inner.p.y - outer.p.y}}, // top
    {{outer.p.x, inner.p.y}, {inner.p.x - outer.p.x, inner.s.h}}, // left
    {{inner.p.x + inner.s.w, inner.p.y}, {(outer.p.x + outer.s.w) - (inner.p.x + inner.s.w), inner.s.h}}, // right
    {{outer.p.x, inner.p.y + inner.s.h}, {outer.s.w, (outer.p.y + outer.s.h) - (inner.p.y + inner.s.h)}} // bottom
  };
  for (const Rect &side : sides) {
    if (!side.empty()) {
      fill(dst.view(side), pixel);
    }
  }
}

template <typename Pixel>
void drawStrokedRect(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Rect rect
) noexcept {
  assert(dst.contains(rect));
  if (rect.s.w <= 2 || rect.s.h <= 2) {
    return fill(dst.view(rect), pixel);
  }
  fillRow(dst, pixel, rect.p, rect.p.x + rect.s.w - 1); // top
  fillCol(dst, pixel, {rect.p.x, rect.p.y + 1}, rect.p.y + rect.s.h - 2); // left
  fillCol(dst, pixel, {rect.p.x + rect.s.w - 2, rect.p.y + 1}, rect.p.y + rect.s.h - 2); // right
  fillRow(dst, pixel, {rect.p.x, rect.p.y + rect.s.h - 1}, rect.p.x + rect.s.w - 1); // bottom
}

// @TODO radius 6

template <typename Pixel>
void drawFilledCircle(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point ctr,
  const int radius,
  const CircleShape shape = CircleShape::c1x1
) noexcept {
  Point pos = {radius, 0};
  int err = 1 - radius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  
  while (pos.x >= pos.y) {
    fillRow(dst, pixel, {ctr.x - pos.x, ctr.y + pos.y + extraY}, ctr.x + pos.x + extraX);
    fillRow(dst, pixel, {ctr.x - pos.x, ctr.y - pos.y},          ctr.x + pos.x + extraX);
    fillRow(dst, pixel, {ctr.x - pos.y, ctr.y + pos.x + extraY}, ctr.x + pos.y + extraX);
    fillRow(dst, pixel, {ctr.x - pos.y, ctr.y - pos.x},          ctr.x + pos.y + extraX);
    
    ++pos.y;
    if (err < 0) {
      err += 2 * pos.y + 1;
    } else {
      --pos.x;
      err += 2 * (pos.y - pos.x) + 1;
    }
  }
}

template <typename Pixel>
void drawStrokedCircle(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point ctr,
  const int radius,
  const CircleShape shape = CircleShape::c1x1
) noexcept {
  assert(radius >= 0);
  Point pos = {radius, 0};
  int err = 1 - radius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  
  while (pos.x >= pos.y) {
    dst.ref({ctr.x + pos.x + extraX, ctr.y + pos.y + extraY}) = pixel;
    dst.ref({ctr.x - pos.x,          ctr.y + pos.y + extraY}) = pixel;
    dst.ref({ctr.x + pos.x + extraX, ctr.y - pos.y         }) = pixel;
    dst.ref({ctr.x - pos.x,          ctr.y - pos.y         }) = pixel;
    
    dst.ref({ctr.x + pos.y + extraX, ctr.y + pos.x + extraY}) = pixel;
    dst.ref({ctr.x - pos.y,          ctr.y + pos.x + extraY}) = pixel;
    dst.ref({ctr.x + pos.y + extraX, ctr.y - pos.x         }) = pixel;
    dst.ref({ctr.x - pos.y,          ctr.y - pos.x         }) = pixel;
    
    ++pos.y;
    if (err < 0) {
      err += 2 * pos.y + 1;
    } else {
      --pos.x;
      err += 2 * (pos.y - pos.x) + 1;
    }
  }
}

template <typename Pixel>
void drawStrokedCircle(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point ctr,
  const int outerRadius,
  const int innerRadius,
  const CircleShape shape = CircleShape::c1x1
) noexcept {
  assert(0 <= outerRadius);
  assert(innerRadius <= outerRadius);
  if (innerRadius <= 0) {
    return drawFilledCircle(dst, pixel, ctr, outerRadius);
  } else if (outerRadius == innerRadius) {
    return drawStrokedCircle(dst, pixel, ctr, outerRadius);
  }
  
  int innerX = innerRadius;
  int outerX = outerRadius;
  int posY = 0;
  int innerErr = 1 - innerRadius;
  int outerErr = 1 - outerRadius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  
  while (outerX >= posY) {
    fillRow(dst, pixel, {ctr.x + innerX + extraX, ctr.y + posY + extraY},   ctr.x + outerX + extraX); // right down
    fillCol(dst, pixel, {ctr.x + posY + extraX,   ctr.y + innerX + extraY}, ctr.y + outerX + extraY); // right down
    fillRow(dst, pixel, {ctr.x - outerX,          ctr.y + posY + extraY},   ctr.x - innerX);          //       down
    fillCol(dst, pixel, {ctr.x - posY,            ctr.y + innerX + extraY}, ctr.y + outerX + extraY); //       down
    
    fillRow(dst, pixel, {ctr.x - outerX,          ctr.y - posY},   ctr.x - innerX);                   //
    fillCol(dst, pixel, {ctr.x - posY,            ctr.y - outerX}, ctr.y - innerX);                   //
    fillRow(dst, pixel, {ctr.x + innerX + extraX, ctr.y - posY},   ctr.x + outerX + extraX);          // right
    fillCol(dst, pixel, {ctr.x + posY + extraX,   ctr.y - outerX}, ctr.y - innerX);                   // right
    
    ++posY;
    if (outerErr < 0) {
      outerErr += 2 * posY + 1;
    } else {
      --outerX;
      outerErr += 2 * (posY - outerX) + 1;
    }
    
    if (posY > innerRadius) {
      innerX = posY;
    } else {
      if (innerErr < 0) {
        innerErr += 2 * posY + 1;
      } else {
        --innerX;
        innerErr += 2 * (posY - innerX) + 1;
      }
    }
  }
}

namespace detail {

std::pair<int, int> signdiff(const int a, const int b) {
  if (a < b) {
    return {1, b - a};
  } else {
    return {-1, a - b};
  }
}

template <typename Func>
void drawLine(Point p1, const Point p2, Func func) {
  // @TODO could be optimized by splitting into four functions
  const auto [sx, dx] = signdiff(p1.x, p2.x);
  auto [sy, dy] = signdiff(p1.y, p2.y);
  dy = -dy;
  int err = dx + dy;
  
  while (true) {
    func(p1);
    const int err2 = 2 * err;
    if (err2 >= dy) {
      if (p1.x == p2.x) break;
      err += dy;
      p1.x += sx;
    }
    if (err2 <= dx) {
      if (p1.y == p2.y) break;
      err += dx;
      p1.y += sy;
    }
  }
}

}

template <typename Pixel>
void drawLine(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point p1,
  const Point p2
) noexcept {
  detail::drawLine(p1, p2, [dst, pixel](const Point pos) {
    dst.ref(pos) = pixel;
  });
}

template <typename Pixel>
void drawLine(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point p1,
  const Point p2,
  const int radius
) noexcept {
  if (radius == 0) {
    drawLine(dst, pixel, p1, p2);
  } else {
    assert(radius > 0);
    // @TODO this is suboptimial but seems to be fast enough
    drawFilledCircle(dst, pixel, p1, radius);
    detail::drawLine(p1, p2, [dst, pixel, radius](const Point pos) {
      drawStrokedCircle(dst, pixel, pos, radius, radius - 1);
    });
  }
}

namespace detail {

template <typename Pixel>
class FillPolicy {
public:
  FillPolicy(const Surface<Pixel> surface, const Pixel fillColor) noexcept
    : surface{surface}, fillColor{fillColor} {}
  
  bool start(const Point pos) noexcept {
    startColor = surface.ref(pos);
    return startColor != fillColor;
  }
  Size size() const noexcept {
    return surface.size();
  }
  bool check(const Point pos) const noexcept {
    return surface.ref(pos) == startColor;
  }
  void set(const Point pos) noexcept {
    surface.ref(pos) = fillColor;
  }

private:
  Surface<Pixel> surface;
  Pixel fillColor;
  Pixel startColor;
};

}

template <typename Pixel>
bool drawFloodFill(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point pos
) noexcept {
  assert(dst.contains(pos));
  return floodFill(detail::FillPolicy{dst, pixel}, pos);
}

}

#endif
