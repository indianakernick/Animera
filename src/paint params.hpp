//
//  paint params.hpp
//  Animera
//
//  Created by Indi Kernick on 21/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef paint_params_hpp
#define paint_params_hpp

#include <QtCore/qrect.h>
#include "enum operators.hpp"

constexpr int no_radius = -1;
constexpr QPoint no_point{-1, -1};
constexpr QSize no_size{-1, -1};

enum class CircleShape {
  c1x1,
  c1x2,
  c2x1,
  c2x2
};

constexpr int centerOffsetX(const CircleShape shape) {
  return (static_cast<std::underlying_type_t<CircleShape>>(shape) & 0b10) >> 1;
}

constexpr int centerOffsetY(const CircleShape shape) {
  return (static_cast<std::underlying_type_t<CircleShape>>(shape) & 0b01);
}

constexpr QRect centerRect(const QPoint point, const CircleShape shape) {
  return {point, QPoint{point.x() + centerOffsetX(shape), point.y() + centerOffsetY(shape)}};
}

enum class SymmetryMode {
  none,
  hori,
  vert,
  both
};

template <>
struct enum_test_flag<SymmetryMode> : std::true_type {};

enum class SelectMode {
  copy,
  paste
};

constexpr SelectMode opposite(const SelectMode mode) {
  return static_cast<SelectMode>(static_cast<std::underlying_type_t<SelectMode>>(mode) ^ 1);
}

#endif
