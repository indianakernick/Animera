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
#include "graphics convert.hpp"

bool drawSquarePoint(QImage &img, const QRgb color, QPoint pos, const gfx::CircleShape shape) {
  pos -= img.offset();
  return visitSurface(img, [color, pos, shape](auto surface) {
    return gfx::drawFilledRect(surface, color, gfx::centerRect(convert(pos), shape));
  });
}

bool drawRoundPoint(QImage &img, const QRgb color, const QPoint pos, const int radius, const gfx::CircleShape shape) {
  assert(radius >= 0);
  if (radius == 0) {
    return drawSquarePoint(img, color, pos, shape);
  } else {
    return drawFilledCircle(img, color, pos, radius, shape);
  }
}

bool drawFloodFill(QImage &img, const QRgb color, QPoint pos, QRect rect) {
  assert(!rect.isEmpty());
  pos -= img.offset();
  if (!img.rect().contains(pos)) return false;
  rect.translate(-img.offset());
  pos -= rect.topLeft();
  return visitSurface(img, [pos, color, rect](auto surface) {
    return gfx::drawFloodFill(surface.view(convert(rect)), color, convert(pos));
  });
}

bool drawFilledCircle(QImage &img, const QRgb color, QPoint center, const int radius, const gfx::CircleShape shape) {
  center -= img.offset();
  return visitSurface(img, [center, radius, shape, color](auto surface) {
    return gfx::drawFilledCircle(surface, color, convert(center), radius, shape);
  });
}

bool drawStrokedCircle(QImage &img, const QRgb color, QPoint center, const int radius, const int thickness, const gfx::CircleShape shape) {
  assert(thickness > 0);
  center -= img.offset();
  return visitSurface(img, [center, radius, thickness, shape, color](auto surface) {
    return gfx::drawStrokedCircle(surface, color, convert(center), radius, radius - thickness + 1, shape);
  });
}

bool drawFilledRect(QImage &img, const QRgb color, QRect rect) {
  rect.translate(-img.offset());
  return visitSurface(img, [rect, color](auto surface) {
    return gfx::drawFilledRect(surface, color, convert(rect));
  });
}

bool drawStrokedRect(QImage &img, const QRgb color, QRect rect, const int thickness) {
  assert(thickness > 0);
  rect.translate(-img.offset());
  if (!img.rect().intersects(rect)) return false;
  return visitSurface(img, [rect, thickness, color](auto surface) {
    const QRect inner = rect.marginsRemoved({thickness, thickness, thickness, thickness});
    return gfx::drawStrokedRect(surface, color, convert(rect), convert(inner));
  });
}

bool drawLine(QImage &img, const QRgb color, QLine line, const int radius) {
  line.translate(-img.offset());
  return visitSurface(img, [line, radius, color](auto surface) {
    return gfx::drawLine(surface, color, convert(line.p1()), convert(line.p2()), radius);
  });
}

bool drawFilledPolygon(
  QImage &img,
  const QRgb color,
  const std::vector<QPoint> &poly
) {
  // TODO: avoid using QPainter
  QPainter painter{&img};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setBrush(QColor::fromRgba(color));
  painter.setPen(QColor::fromRgba(color));
  painter.drawPolygon(poly.data(), static_cast<int>(poly.size()));
  return true;
}
