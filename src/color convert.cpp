//
//  color convert.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color convert.hpp"

#include <cmath>

QRgb hsv2rgb(const qreal h, const qreal s, const qreal v) {
  QColor color;
  color.setHsvF(h / 360.0, s / 100.0, v / 100.0);
  return color.rgba();
}

HSV color2hsv(const QColor color) {
  const QColor hsvColor = color.toHsv();
  qreal h, s, v;
  hsvColor.getHsvF(&h, &s, &v);
  return {
    qRound(h * 360.0) % 360,
    qRound(s * 100.0),
    qRound(v * 100.0)
  };
}

RGB color2rgb(const QColor color) {
  const QColor rgbColor = color.toRgb();
  RGB rgb;
  rgbColor.getRgb(&rgb.r, &rgb.g, &rgb.b);
  return rgb;
}

RGB hsv2rgb(const HSV hsv) {
  QColor color;
  color.setHsvF(hsv.h / 360.0, hsv.s / 100.0, hsv.v / 100.0);
  return color2rgb(color);
}

HSV rgb2hsv(const RGB rgb) {
  QColor color;
  color.setRgb(rgb.r, rgb.g, rgb.b);
  return color2hsv(color);
}
