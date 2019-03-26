//
//  utils.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 26/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef utils_hpp
#define utils_hpp

#include <QtGui/qcolor.h>
#include <QtCore/qsize.h>
#include <QtCore/qpoint.h>

constexpr QSize toSize(const QPoint p) {
  return {p.x(), p.y()};
}

constexpr QPoint toPoint(const QSize s) {
  return {s.width(), s.height()};
}

inline QColor toColor(const QRgb rgba) {
  // the QRgb constructor sets alpha to 255 for some reason
  return QColor{qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba)};
}

#endif
