//
//  graphics convert.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/9/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_graphics_convert_hpp
#define animera_graphics_convert_hpp

#include <QtCore/qrect.h>
#include <Graphics/geometry.hpp>

constexpr gfx::Point convert(const QPoint p) {
  return {p.x(), p.y()};
}

constexpr gfx::Size convert(const QSize s) {
  return {s.width(), s.height()};
}

constexpr gfx::Rect convert(const QRect r) {
  return {{r.x(), r.y()}, {r.width(), r.height()}};
}

constexpr QPoint convert(const gfx::Point p) {
  return {p.x, p.y};
}

constexpr QSize convert(const gfx::Size s) {
  return {s.w, s.h};
}

constexpr QRect convert(const gfx::Rect r) {
  return {r.p.x, r.p.y, r.s.w, r.s.h};
}

#endif
