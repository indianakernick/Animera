//
//  color convert.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color convert.hpp"

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
    qRound(h * 359),
    qRound(s * 100),
    qRound(v * 100)
  };
}
