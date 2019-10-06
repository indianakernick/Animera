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
#include <Graphics/geometry.hpp>

constexpr int no_radius = -1;
constexpr QPoint no_point{-1, -1};
constexpr QSize no_size{-1, -1};

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
  return static_cast<SelectMode>(static_cast<int>(mode) ^ 1);
}

enum class LineGradMode {
  hori,
  vert
};

constexpr LineGradMode opposite(const LineGradMode mode) {
  return static_cast<LineGradMode>(static_cast<int>(mode) ^ 1);
}

#endif
