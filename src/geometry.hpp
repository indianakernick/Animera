//
//  geometry.hpp
//  Animera
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef geometry_hpp
#define geometry_hpp

#include <QtCore/qrect.h>

constexpr QSize toSize(const QPoint p) {
  return {p.x(), p.y()};
}

constexpr QSize toSize(const int i) {
  return {i, i};
}

constexpr QPoint toPoint(const QSize s) {
  return {s.width(), s.height()};
}

constexpr QPoint toPoint(const int i) {
  return {i, i};
}

constexpr QRect toRect(const QPoint p) {
  return {p, QSize{1, 1}};
}

constexpr QRect toRect(const QSize s) {
  return {{0, 0}, s};
}

constexpr QRect unite(QPoint a, QPoint b) {
  // Can't use QRect{a, b}.normalized()
  const auto [minX, maxX] = std::minmax(a.rx(), b.rx());
  const auto [minY, maxY] = std::minmax(a.ry(), b.ry());
  return {QPoint{minX, minY}, QPoint{maxX, maxY}};
}

#endif
