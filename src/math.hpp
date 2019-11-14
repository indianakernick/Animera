//
//  math.hpp
//  Animera
//
//  Created by Indi Kernick on 14/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef math_hpp
#define math_hpp

constexpr int scaleImpl(
  const int value,
  const int fromMin,
  const int fromMax,
  const int toMin,
  const int toMax,
  const int rounding
) {
  const int toSize = toMax - toMin;
  const int fromSize = fromMax - fromMin;
  return ((value - fromMin) * toSize + rounding) / fromSize + toMin;
}

constexpr int scaleF(
  const int value,
  const int fromMin,
  const int fromMax,
  const int toMin,
  const int toMax
) {
  return scaleImpl(value, fromMin, fromMax, toMin, toMax, 0);
}

constexpr int scaleC(
  const int value,
  const int fromMin,
  const int fromMax,
  const int toMin,
  const int toMax
) {
  return scaleImpl(value, fromMin, fromMax, toMin, toMax, fromMax - fromMin - 1);
}

constexpr int scale(
  const int value,
  const int fromMin,
  const int fromMax,
  const int toMin,
  const int toMax
) {
  return scaleImpl(value, fromMin, fromMax, toMin, toMax, (fromMax - fromMin) / 2);
}

constexpr int scaleF(const int value, const int fromMax, const int toMax) {
  return (value * toMax) / fromMax;
}

constexpr int scaleC(const int value, const int fromMax, const int toMax) {
  return (value * toMax + fromMax - 1) / fromMax;
}

constexpr int scale(const int value, const int fromMax, const int toMax) {
  return (value * toMax + fromMax / 2) / fromMax;
}

#endif
