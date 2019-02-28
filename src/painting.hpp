//
//  painting.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 21/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef painting_hpp
#define painting_hpp

#include <QtGui/qimage.h>
#include "paint params.hpp"

QRect circleToRect(QPoint, int, CircleShape);
QRect centerToRect(QPoint, CircleShape);

bool drawSquarePoint   (QImage &, QRgb, QPoint);
bool drawRoundPoint    (QImage &, QRgb, QPoint, int);

bool drawFloodFill     (QImage &, QRgb, QPoint);

bool drawFilledEllipse (QImage &, QRgb, QRect);
bool drawStrokedEllipse(QImage &, QRgb, QRect);

bool drawFilledRect    (QImage &, QRgb, QRect);
bool drawStrokedRect   (QImage &, QRgb, QRect);

bool drawLine          (QImage &, QRgb, QLine);
bool drawRoundLine     (QImage &, QRgb, QLine, int);

bool drawFilledPolygon(QImage &, QRgb, const std::vector<QPoint> &, QPoint);

#endif
