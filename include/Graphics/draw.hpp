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
bool drawPoint(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point pos
) noexcept {
  if (dst.contains(pos)) {
    dst.ref(pos) = pixel;
    return true;
  }
  return false;
}

template <typename Pixel>
bool drawFilledRect(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Rect rect
) noexcept {
  return fillRegion(dst, pixel, rect);
}

template <typename Pixel>
bool drawStrokedRect(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  Rect outer,
  Rect inner
) noexcept {
  outer = outer.intersected(dst.rect());
  inner = inner.intersected(dst.rect());
  assert(inner.empty() || outer.contains(inner));
  if (outer.empty()) {
    return false;
  }
  if (inner.empty()) {
    fill(dst.view(outer), pixel);
    return true;
  }
  const Rect sides[4] = {
    { // top
      outer.p,
      {outer.s.w, inner.p.y - outer.p.y}
    },
    { // left
      {outer.p.x, inner.p.y},
      {inner.p.x - outer.p.x, inner.s.h}
    },
    { // right
      {inner.p.x + inner.s.w, inner.p.y},
      {(outer.p.x + outer.s.w) - (inner.p.x + inner.s.w), inner.s.h}
    },
    { // bottom
      {outer.p.x, inner.p.y + inner.s.h},
      {outer.s.w, (outer.p.y + outer.s.h) - (inner.p.y + inner.s.h)}
    }
  };
  bool drawn = false;
  for (const Rect &side : sides) {
    if (!side.empty()) {
      fill(dst.view(side), pixel);
      drawn = true;
    }
  }
  return drawn;
}

template <typename Pixel>
bool drawStrokedRect(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Rect rect
) noexcept {
  bool drawn = false;
  drawn |= fillRowRegion(dst, pixel, // top
    rect.p,
    rect.p.x + rect.s.w - 1
  );
  drawn |= fillColRegion(dst, pixel, // left
    {rect.p.x, rect.p.y + 1},
    rect.p.y + rect.s.h - 2
  );
  drawn |= fillColRegion(dst, pixel, // right
    {rect.p.x + rect.s.w - 2, rect.p.y + 1},
    rect.p.y + rect.s.h - 2
  );
  drawn |= fillRowRegion(dst, pixel, // bottom
    {rect.p.x, rect.p.y + rect.s.h - 1},
    rect.p.x + rect.s.w - 1
  );
  return drawn;
}

// TODO: radius 6

template <typename Pixel>
bool drawFilledCircle(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point ctr,
  const int radius,
  const CircleShape shape = CircleShape::c1x1
) noexcept {
  assert(radius >= 0);
  if (circleRect(ctr, radius).intersected(dst.rect()).empty()) return false;
  Point pos = {radius, 0};
  int err = 1 - radius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  bool drawn = false;
  
  while (pos.x >= pos.y) {
    drawn |= fillRowRegion(dst, pixel,
      {ctr.x - pos.x, ctr.y + pos.y + extraY},
      ctr.x + pos.x + extraX
    );
    drawn |= fillRowRegion(dst, pixel,
      {ctr.x - pos.x, ctr.y - pos.y},
      ctr.x + pos.x + extraX);
    drawn |= fillRowRegion(dst, pixel,
      {ctr.x - pos.y, ctr.y + pos.x + extraY},
      ctr.x + pos.y + extraX
    );
    drawn |= fillRowRegion(dst, pixel,
      {ctr.x - pos.y, ctr.y - pos.x},
      ctr.x + pos.y + extraX
    );
    
    ++pos.y;
    if (err < 0) {
      err += 2 * pos.y + 1;
    } else {
      --pos.x;
      err += 2 * (pos.y - pos.x) + 1;
    }
  }
  
  return drawn;
}

template <typename Pixel>
bool drawStrokedCircle(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point ctr,
  const int radius,
  const CircleShape shape = CircleShape::c1x1
) noexcept {
  assert(radius >= 0);
  if (circleRect(ctr, radius).intersected(dst.rect()).empty()) return false;
  Point pos = {radius, 0};
  int err = 1 - radius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  bool drawn = false;
  
  while (pos.x >= pos.y) {
    drawn |= drawPoint(dst, pixel, {ctr.x + pos.x + extraX, ctr.y + pos.y + extraY});
    drawn |= drawPoint(dst, pixel, {ctr.x - pos.x,          ctr.y + pos.y + extraY});
    drawn |= drawPoint(dst, pixel, {ctr.x + pos.x + extraX, ctr.y - pos.y         });
    drawn |= drawPoint(dst, pixel, {ctr.x - pos.x,          ctr.y - pos.y         });
    
    drawn |= drawPoint(dst, pixel, {ctr.x + pos.y + extraX, ctr.y + pos.x + extraY});
    drawn |= drawPoint(dst, pixel, {ctr.x - pos.y,          ctr.y + pos.x + extraY});
    drawn |= drawPoint(dst, pixel, {ctr.x + pos.y + extraX, ctr.y - pos.x         });
    drawn |= drawPoint(dst, pixel, {ctr.x - pos.y,          ctr.y - pos.x         });
    
    ++pos.y;
    if (err < 0) {
      err += 2 * pos.y + 1;
    } else {
      --pos.x;
      err += 2 * (pos.y - pos.x) + 1;
    }
  }
  
  return drawn;
}

template <typename Pixel>
bool drawStrokedCircle(
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
    return drawFilledCircle(dst, pixel, ctr, outerRadius, shape);
  } else if (outerRadius == innerRadius) {
    return drawStrokedCircle(dst, pixel, ctr, outerRadius, shape);
  }
  
  int innerX = innerRadius;
  int outerX = outerRadius;
  int posY = 0;
  int innerErr = 1 - innerRadius;
  int outerErr = 1 - outerRadius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  bool drawn = false;
  
  while (outerX >= posY) {
    drawn |= fillRowRegion(dst, pixel, // right down
      {ctr.x + innerX + extraX, ctr.y + posY + extraY},
      ctr.x + outerX + extraX
    );
    drawn |= fillColRegion(dst, pixel, // right down
      {ctr.x + posY + extraX, ctr.y + innerX + extraY},
      ctr.y + outerX + extraY
    );
    drawn |= fillRowRegion(dst, pixel, //       down
      {ctr.x - outerX, ctr.y + posY + extraY},
      ctr.x - innerX
    );
    drawn |= fillColRegion(dst, pixel, //       down
      {ctr.x - posY, ctr.y + innerX + extraY},
      ctr.y + outerX + extraY
    );
    
    drawn |= fillRowRegion(dst, pixel,
      {ctr.x - outerX, ctr.y - posY},
      ctr.x - innerX
    );
    drawn |= fillColRegion(dst, pixel,
      {ctr.x - posY, ctr.y - outerX},
      ctr.y - innerX
    );
    drawn |= fillRowRegion(dst, pixel, // right
      {ctr.x + innerX + extraX, ctr.y - posY},
      ctr.x + outerX + extraX
    );
    drawn |= fillColRegion(dst, pixel, // right
      {ctr.x + posY + extraX, ctr.y - outerX},
      ctr.y - innerX
    );
    
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
  
  return drawn;
}

namespace detail {

inline std::pair<int, int> signdiff(const int a, const int b) {
  if (a < b) {
    return {1, b - a};
  } else {
    return {-1, a - b};
  }
}

template <typename Func>
bool drawLine(Point p1, const Point p2, Func func) {
  // TODO: could be optimized by splitting into four functions
  const auto [sx, dx] = signdiff(p1.x, p2.x);
  auto [sy, dy] = signdiff(p1.y, p2.y);
  dy = -dy;
  int err = dx + dy;
  bool drawn = false;
  
  while (true) {
    drawn |= func(p1);
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
  
  return drawn;
}

}

template <typename Pixel>
bool drawLine(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point p1,
  const Point p2
) noexcept {
  return detail::drawLine(p1, p2, [dst, pixel](const Point pos) {
    return drawPoint(dst, pixel, pos);
  });
}

template <typename Pixel>
bool drawLine(
  const Surface<Pixel> dst,
  const identity_t<Pixel> pixel,
  const Point p1,
  const Point p2,
  const int radius
) noexcept {
  if (radius == 0) {
    return drawLine(dst, pixel, p1, p2);
  } else {
    assert(radius > 0);
    // TODO: this is suboptimial but seems to be fast enough
    bool drawn = drawFilledCircle(dst, pixel, p1, radius);
    drawn |= detail::drawLine(p1, p2, [dst, pixel, radius](const Point pos) {
      return drawStrokedCircle(dst, pixel, pos, radius, radius - 1);
    });
    return drawn;
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
  if (dst.contains(pos)) {
    return !floodFill(detail::FillPolicy{dst, pixel}, pos).empty();
  }
  return false;
}

template <typename Pixel, typename Func>
bool drawHoriGradient(
  const Surface<Pixel> dst,
  const Rect rect,
  Func func
) noexcept {
  if (rect.empty()) return false;
  const int bottom = rect.p.y + rect.s.h - 1;
  const int size = rect.s.w == 1 ? 1 : rect.s.w - 1;
  bool drawn = false;
  
  for (int x = 0; x != rect.s.w; ++x) {
    const Pixel pixel = func(x, size);
    drawn |= fillColRegion(dst, pixel, {rect.p.x + x, rect.p.y}, bottom);
  }
  
  return drawn;
}

template <typename Pixel, typename Func>
bool drawVertGradient(
  const Surface<Pixel> dst,
  const Rect rect,
  Func func
) noexcept {
  if (rect.empty()) return false;
  const int right = rect.p.x + rect.s.w - 1;
  const int size = rect.s.h == 1 ? 1 : rect.s.h - 1;
  bool drawn = false;
  
  for (int y = 0; y != rect.s.h; ++y) {
    const Pixel pixel = func(y, size);
    drawn |= fillRowRegion(dst, pixel, {rect.p.x, rect.p.y + y}, right);
  }
  
  return drawn;
}

}

#endif
