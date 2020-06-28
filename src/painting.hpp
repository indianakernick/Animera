//
//  painting.hpp
//  Animera
//
//  Created by Indiana Kernick on 21/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_painting_hpp
#define animera_painting_hpp

#include "image.hpp"
#include "paint params.hpp"

/*
Why don't you use QPainter?

Qt doesn't have a flood fill function.

Qt seems to have trouble drawing circles.
https://stackoverflow.com/questions/54776781/drawing-pixel-perfect-circles-with-qt

Everything else is there for consistency and because it's fun to implement.
*/

bool drawSquarePoint  (QImage &, PixelVar, QPoint, gfx::CircleShape = gfx::CircleShape::c1x1);
bool drawRoundPoint   (QImage &, PixelVar, QPoint, int = 1, gfx::CircleShape = gfx::CircleShape::c1x1);

bool drawFloodFill    (QImage &, PixelVar, QPoint, QRect);

bool drawFilledCircle (QImage &, PixelVar, QPoint, int, gfx::CircleShape = gfx::CircleShape::c1x1);
bool drawStrokedCircle(QImage &, PixelVar, QPoint, int, int = 1, gfx::CircleShape = gfx::CircleShape::c1x1);

bool drawFilledRect   (QImage &, PixelVar, QRect);
bool drawStrokedRect  (QImage &, PixelVar, QRect, int = 1);

bool drawHoriGradient (QImage &, PixelVar, PixelVar, QRect);
bool drawVertGradient (QImage &, PixelVar, PixelVar, QRect);

bool drawLine         (QImage &, PixelVar, QLine, int = 1);

bool drawFilledPolygon(QImage &, QRgb, const std::vector<QPoint> &);

#endif
