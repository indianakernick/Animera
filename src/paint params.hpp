//
//  paint params.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 21/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef paint_params_hpp
#define paint_params_hpp

#include <QtCore/qsize.h>
#include <QtCore/qpoint.h>

constexpr int min_thickness = 1;
constexpr int max_thickness = 64;
constexpr int no_radius = -1;
constexpr QPoint no_point{-1, -1};
constexpr QSize no_size{-1, -1};

enum class CircleShape {
  c1x1,
  c1x2,
  c2x1,
  c2x2
};

#endif
