//
//  painting.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 21/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef painting_hpp
#define painting_hpp

#include "polygon.hpp"
#include <QtGui/qimage.h>
#include "paint params.hpp"

bool drawSquarePoint  (QImage &, QRgb, QPoint, CircleShape = CircleShape::c1x1);
bool drawRoundPoint   (QImage &, QRgb, QPoint, int = 1, CircleShape = CircleShape::c1x1);

bool drawFloodFill    (QImage &, QRgb, QPoint);

bool drawFilledCircle (QImage &, QRgb, QPoint, int, CircleShape = CircleShape::c1x1);
bool drawStrokedCircle(QImage &, QRgb, QPoint, int, int = 1, CircleShape = CircleShape::c1x1);

bool drawFilledRect   (QImage &, QRgb, QRect);
bool drawStrokedRect  (QImage &, QRgb, QRect, int = 1);

bool drawLine         (QImage &, QRgb, QLine, int = 1);

bool drawFilledPolygon(QImage &, QRgb, const Polygon &, QPoint);

#endif
