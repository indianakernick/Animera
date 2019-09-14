//
//  painting.cpp
//  Animera
//
//  Created by Indi Kernick on 21/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "painting.hpp"

#include "geometry.hpp"
#include <QtGui/qpainter.h>
#include <Graphics/draw.hpp>
#include "surface factory.hpp"

bool drawSquarePoint(QImage &img, const QRgb color, const QPoint pos, const gfx::CircleShape shape) {
  visitSurface(img, [color, pos, shape](auto surface) {
    gfx::drawFilledRect(surface, color, gfx::centerRect(convert(pos), shape));
  });
  return true;
}

bool drawRoundPoint(QImage &img, const QRgb color, const QPoint pos, const int radius, const gfx::CircleShape shape) {
  assert(radius >= 0);
  if (radius == 0) {
    return drawSquarePoint(img, color, pos, shape);
  } else {
    return drawFilledCircle(img, color, pos, radius, shape);
  }
}

bool drawFloodFill(QImage &img, const QRgb color, const QPoint pos) {
  if (!img.rect().contains(pos)) return false;
  return visitSurface(img, [pos, color](auto surface) {
    return gfx::drawFloodFill(surface, color, convert(pos));
  });
}

bool drawFilledCircle(QImage &img, const QRgb color, const QPoint center, const int radius, const gfx::CircleShape shape) {
  visitSurface(img, [center, radius, shape, color](auto surface) {
    gfx::drawFilledCircle(surface, color, convert(center), radius, shape);
  });
  return true;
}

bool drawStrokedCircle(QImage &img, const QRgb color, const QPoint center, const int radius, const int thickness, const gfx::CircleShape shape) {
  assert(thickness > 0);
  visitSurface(img, [center, radius, thickness, shape, color](auto surface) {
    gfx::drawStrokedCircle(surface, color, convert(center), radius, radius - thickness + 1, shape);
  });
  return true;
}

bool drawFilledRect(QImage &img, const QRgb color, const QRect rect) {
  visitSurface(img, [rect, color](auto surface) {
    gfx::drawFilledRect(surface, color, convert(rect));
  });
  return true;
}

bool drawStrokedRect(QImage &img, const QRgb color, const QRect rect, const int thickness) {
  assert(thickness > 0);
  if (!img.rect().intersects(rect)) return false;
  visitSurface(img, [rect, thickness, color](auto surface) {
    const QRect inner = rect.marginsRemoved({thickness, thickness, thickness, thickness});
    gfx::drawStrokedRect(surface, color, convert(rect), convert(inner));
  });
  return true;
}

bool drawLine(QImage &img, const QRgb color, const QLine line, const int radius) {
  visitSurface(img, [line, radius, color](auto surface) {
    gfx::drawLine(surface, color, convert(line.p1()), convert(line.p2()), radius);
  });
  return true;
}

bool drawFilledPolygon(
  QImage &img,
  const QRgb color,
  const Polygon &poly,
  const QPoint offset
) {
  std::vector<QPoint> shiftedPoly;
  shiftedPoly.reserve(poly.size());
  for (const QPoint &vertex : poly) {
    shiftedPoly.push_back(vertex + offset);
  }
  // @TODO avoid using QPainter
  QPainter painter{&img};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setBrush(QColor::fromRgba(color));
  painter.setPen(QColor::fromRgba(color));
  painter.drawPolygon(shiftedPoly.data(), static_cast<int>(shiftedPoly.size()));
  return true;
}
