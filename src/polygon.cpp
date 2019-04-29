//
//  polygon.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 26/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "polygon.hpp"

#include "geometry.hpp"

void Polygon::init(const QPoint point) {
  points.clear();
  points.push_back(point);
  rect = toRect(point);
}

void Polygon::push(const QPoint point) {
  assert(!points.empty());
  if (points.back() == point) return;
  points.push_back(point);
  rect = rect.united(toRect(point));
}
