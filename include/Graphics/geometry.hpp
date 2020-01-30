//
//  geometry.hpp
//  Animera
//
//  Created by Indi Kernick on 1/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_geometry_hpp
#define graphics_geometry_hpp

#include <cstddef>

namespace gfx {

struct Size;

struct Point {
  int x = 0;
  int y = 0;
  
  constexpr Size size() const noexcept;
  
  constexpr Point transposed() const noexcept {
    return {y, x};
  }
  
  constexpr Point operator-() const noexcept {
    return {-x, -y};
  }
  
  constexpr Point operator+(const Point other) const noexcept {
    return {x + other.x, y + other.y};
  }
  constexpr Point operator-(const Point other) const noexcept {
    return {x - other.x, y - other.y};
  }
  
  constexpr Point operator+(const int value) const noexcept {
    return {x + value, y + value};
  }
  constexpr Point operator-(const int value) const noexcept {
    return {x - value, y - value};
  }
  constexpr Point operator*(const int value) const noexcept {
    return {x * value, y * value};
  }
  constexpr Point operator/(const int value) const noexcept {
    return {x / value, y / value};
  }
  
  constexpr Point &operator+=(const Point other) noexcept {
    x += other.x;
    y += other.y;
    return *this;
  }
  constexpr Point &operator-=(const Point other) noexcept {
    x -= other.x;
    y -= other.y;
    return *this;
  }
  
  constexpr Point &operator+=(const int value) noexcept {
    x += value;
    y += value;
    return *this;
  }
  constexpr Point &operator-=(const int value) noexcept {
    x -= value;
    y -= value;
    return *this;
  }
  constexpr Point &operator*=(const int value) noexcept {
    x *= value;
    y *= value;
    return *this;
  }
  constexpr Point &operator/=(const int value) noexcept {
    x /= value;
    y /= value;
    return *this;
  }
  
  constexpr bool operator==(const Point other) const noexcept {
    return x == other.x && y == other.y;
  }
  constexpr bool operator!=(const Point other) const noexcept {
    return !(*this == other);
  }
};

struct Size {
  int w = 0;
  int h = 0;
  
  constexpr Point point() const noexcept {
    return {w, h};
  }
  
  constexpr bool empty() const noexcept {
    return w <= 0 || h <= 0;
  }
  constexpr Size transposed() const noexcept {
    return {h, w};
  }
  
  constexpr Size operator+(const Size other) const noexcept {
    return {w + other.w, h + other.h};
  }
  constexpr Size operator-(const Size other) const noexcept {
    return {w - other.w, h - other.h};
  }
  
  constexpr Size operator+(const int value) const noexcept {
    return {w + value, h + value};
  }
  constexpr Size operator-(const int value) const noexcept {
    return {w - value, h - value};
  }
  constexpr Size operator*(const int value) const noexcept {
    return {w * value, h * value};
  }
  constexpr Size operator/(const int value) const noexcept {
    return {w / value, h / value};
  }
  
  constexpr bool operator==(const Size other) const noexcept {
    return w == other.w && h == other.h;
  }
  constexpr bool operator!=(const Size other) const noexcept {
    return !(*this == other);
  }
};

constexpr Size Point::size() const noexcept {
  return {x, y};
}

struct Rect {
  Point p{};
  Size s{};
  
  constexpr Point topLeft() const noexcept {
    return p;
  }
  constexpr Point topRight() const noexcept {
    return {p.x + s.w, p.y};
  }
  constexpr Point bottomRight() const noexcept {
    return {p.x + s.w, p.y + s.h};
  }
  constexpr Point bottomLeft() const noexcept {
    return {p.x, p.y + s.h};
  }
  
  constexpr bool empty() const noexcept {
    return s.empty();
  }
  constexpr bool contains(const Point pos) const noexcept {
    return p.x <= pos.x &&
           p.y <= pos.y &&
           pos.x < p.x + s.w &&
           pos.y < p.y + s.h;
  }
  constexpr bool contains(const Rect rect) const noexcept {
    return !rect.empty() &&
           contains(rect.p) &&
           rect.p.x + rect.s.w <= p.x + s.w &&
           rect.p.y + rect.s.h <= p.y + s.h;
  }
  
  constexpr Rect intersected(const Rect other) const noexcept {
    if (empty() || other.empty()) return {};
    
    // <algorithm> header is about 20k lines
    const auto max = [](const int a, const int b) {
      return a < b ? b : a;
    };
    const auto min = [](const int a, const int b) {
      return a < b ? a : b;
    };
    
    const Point topLeft = {
      max(p.x, other.p.x),
      max(p.y, other.p.y)
    };
    const Point bottomRight = {
      min(p.x + s.w, other.p.x + other.s.w),
      min(p.y + s.h, other.p.y + other.s.h)
    };
    
    if (bottomRight.x > topLeft.x && bottomRight.y > topLeft.y) {
      return {topLeft, (bottomRight - topLeft).size()};
    } else {
      return {};
    }
  }
  
  constexpr bool intersects(const Rect other) const noexcept {
    return !intersected(other).empty();
  }
  
  bool operator==(const Rect other) const noexcept {
    return p == other.p && s == other.s;
  }
  bool operator!=(const Rect other) const noexcept {
    return !(*this == other);
  }
};

enum class CircleShape {
  c1x1,
  c1x2,
  c2x1,
  c2x2
};

constexpr int centerOffsetX(const CircleShape shape) {
  return (static_cast<int>(shape) & 0b10) >> 1;
}

constexpr int centerOffsetY(const CircleShape shape) {
  return (static_cast<int>(shape) & 0b01);
}

constexpr Rect centerRect(const Point point, const CircleShape shape) {
  return {point, {1 + centerOffsetX(shape), 1 + centerOffsetY(shape)}};
}

constexpr Rect circleRect(const Point ctr, const int rad, const CircleShape shape = gfx::CircleShape::c1x1) {
  return {
    {ctr.x - rad, ctr.y - rad},
    {2 * rad + 1 + centerOffsetX(shape), 2 * rad + 1 + centerOffsetY(shape)}
  };
}

}

namespace std {

static_assert(sizeof(size_t) == 8 && sizeof(int) == 4);

template <>
struct hash<gfx::Point> {
  size_t operator()(const gfx::Point point) const noexcept {
    return (static_cast<size_t>(point.x) << 32) | point.y;
  }
};

template <>
struct hash<gfx::Size> {
  size_t operator()(const gfx::Size size) const noexcept {
    return (static_cast<size_t>(size.w) << 32) | size.h;
  }
};

}

#endif
