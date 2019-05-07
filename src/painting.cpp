//
//  painting.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 21/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "painting.hpp"

#include "geometry.hpp"
#include <QtGui/qpainter.h>
#include "surface factory.hpp"

// @TODO naming and parameter order are not very consistent in this file

bool drawSquarePoint(QImage &img, const QRgb color, const QPoint pos, const CircleShape shape) {
  if (img.depth() == 8) {
    return makeSurface<uint8_t>(img).fillRectClip(color, centerRect(pos, shape));
  } else if (img.depth() == 32) {
    return makeSurface<uint32_t>(img).fillRectClip(color, centerRect(pos, shape));
  } else {
    Q_UNREACHABLE();
  }
}

bool drawRoundPoint(QImage &img, const QRgb color, const QPoint pos, const int thickness, const CircleShape shape) {
  assert(thickness > 0);
  if (thickness == 1) {
    return drawSquarePoint(img, color, pos, shape);
  } else {
    return drawFilledCircle(img, color, pos, thickness, shape);
  }
}

namespace {

template <typename Pixel>
class FloodFillSurface {
public:
  FloodFillSurface(const Surface<Pixel> &surface, const Pixel startColor, const Pixel toolColor)
    : surface{surface},
      startColor{startColor},
      toolColor{toolColor} {}

  bool filled(const QPoint pos) const {
    return surface.getPixel(pos) != startColor;
  }
  void fill(const QPoint pos) {
    surface.setPixel(toolColor, pos);
  }

private:
  Surface<Pixel> surface;
  Pixel startColor;
  Pixel toolColor;
};

QPoint up(const QPoint p) {
  return {p.x(), p.y() - 1};
}

QPoint right(const QPoint p) {
  return {p.x() + 1, p.y()};
}

QPoint down(const QPoint p) {
  return {p.x(), p.y() + 1};
}

QPoint left(const QPoint p) {
  return {p.x() - 1, p.y()};
}

// Flood Fill algorithm by Adam Milazzo
// http://www.adammil.net/blog/v126_A_More_Efficient_Flood_Fill.html

template <typename Pixel>
void floodFillStart(FloodFillSurface<Pixel>, QPoint, QSize);

template <typename Pixel>
void floodFillCore(FloodFillSurface<Pixel> px, QPoint pos, const QSize size) {
  int lastRowLength = 0;
  do {
    int rowLength = 0;
    QPoint start = pos;
    if (lastRowLength != 0 && px.filled(pos)) {
      do {
        if (--lastRowLength == 0) return;
        pos = right(pos);
      } while (px.filled(pos));
    } else {
      while (pos.x() != 0 && !px.filled(left(pos))) {
        pos = left(pos);
        px.fill(pos);
        if (pos.y() != 0 && !px.filled(up(pos))) floodFillStart(px, up(pos), size);
        ++rowLength;
        ++lastRowLength;
      }
    }
    while (start.x() < size.width() && !px.filled(start)) {
      px.fill(start);
      start = right(start);
      ++rowLength;
    }
    if (rowLength < lastRowLength) {
      const int endX = pos.x() + lastRowLength;
      while (++start.rx() < endX) {
        if (!px.filled(start)) floodFillCore(px, start, size);
      }
    } else if (rowLength > lastRowLength && pos.y() != 0) {
      QPoint above = up({pos.x() + lastRowLength, pos.y()});
      while (++above.rx() < start.x()) {
        if (!px.filled(above)) floodFillStart(px, above, size);
      }
    }
    lastRowLength = rowLength;
    pos = down(pos);
  } while (lastRowLength != 0 && pos.y() < size.height());
}

template <typename Pixel>
void floodFillStart(FloodFillSurface<Pixel> px, QPoint pos, const QSize size) {
  while (true) {
    const QPoint startPos = pos;
    while (pos.y() != 0 && !px.filled(up(pos))) pos = up(pos);
    while (pos.x() != 0 && !px.filled(left(pos))) pos = left(pos);
    if (pos == startPos) break;
  }
  floodFillCore(px, pos, size);
}

template <typename Pixel>
bool floodFill(QImage &img, const QPoint startPos, const Pixel color) {
  img.detach();
  Surface surface = makeSurface<Pixel>(img);
  const Pixel toolColor = static_cast<Pixel>(color);
  const Pixel startColor = surface.getPixel(startPos);
  if (startColor == toolColor) return false;
  FloodFillSurface<Pixel> px{surface, startColor, toolColor};
  floodFillStart(px, startPos, img.size());
  return true;
}

}

bool drawFloodFill(QImage &img, const QRgb color, const QPoint pos) {
  if (!img.rect().contains(pos)) return false;
  if (img.depth() == 8) {
    return floodFill<uint8_t>(img, pos, color);
  } else if (img.depth() == 32) {
    return floodFill<uint32_t>(img, pos, color);
  } else {
    Q_UNREACHABLE();
  }
}

// @TODO radius 6

namespace {

template <typename Pixel>
bool midpointFilledCircle(
  QImage &img,
  const Pixel col,
  const QPoint ctr,
  const int rad,
  const CircleShape shape
) {
  Surface surface = makeSurface<Pixel>(img);
  QPoint pos = {rad, 0};
  int err = 1 - rad;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  bool drawn = false;
  
  while (pos.x() >= pos.y()) {
    drawn |= surface.horiLineClip(col, {ctr.x() - pos.x(), ctr.y() + pos.y() + extraY}, ctr.x() + pos.x() + extraX);
    drawn |= surface.horiLineClip(col, {ctr.x() - pos.x(), ctr.y() - pos.y()},          ctr.x() + pos.x() + extraX);
    drawn |= surface.horiLineClip(col, {ctr.x() - pos.y(), ctr.y() + pos.x() + extraY}, ctr.x() + pos.y() + extraX);
    drawn |= surface.horiLineClip(col, {ctr.x() - pos.y(), ctr.y() - pos.x()},          ctr.x() + pos.y() + extraX);
    
    ++pos.ry();
    
    if (err < 0) {
      err += 2 * pos.y() + 1;
    } else {
      --pos.rx();
      err += 2 * (pos.y() - pos.x()) + 1;
    }
  }
  
  return drawn;
}

}

bool drawFilledCircle(QImage &img, const QRgb color, const QPoint center, const int radius, const CircleShape shape) {
  if (img.depth() == 8) {
    return midpointFilledCircle<uint8_t>(img, color, center, radius, shape);
  } else if (img.depth() == 32) {
    return midpointFilledCircle<uint32_t>(img, color, center, radius, shape);
  } else {
    Q_UNREACHABLE();
  }
}

namespace {

template <typename Pixel>
bool midpointCircle(
  QImage &image,
  Pixel color,
  QPoint center,
  int radius,
  CircleShape shape
) {
  Surface surface = makeSurface<Pixel>(image);
  QPoint pos = {radius, 0};
  int err = 1 - radius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  bool drawn = false;
  
  while (pos.x() >= pos.y()) {
    drawn |= surface.setPixelClip(color, {center.x() + pos.x() + extraX, center.y() + pos.y() + extraY});
    drawn |= surface.setPixelClip(color, {center.x() - pos.x(),          center.y() + pos.y() + extraY});
    drawn |= surface.setPixelClip(color, {center.x() + pos.x() + extraX, center.y() - pos.y()});
    drawn |= surface.setPixelClip(color, {center.x() - pos.x(),          center.y() - pos.y()});
    
    drawn |= surface.setPixelClip(color, {center.x() + pos.y() + extraX, center.y() + pos.x() + extraY});
    drawn |= surface.setPixelClip(color, {center.x() - pos.y(),          center.y() + pos.x() + extraY});
    drawn |= surface.setPixelClip(color, {center.x() + pos.y() + extraX, center.y() - pos.x()});
    drawn |= surface.setPixelClip(color, {center.x() - pos.y(),          center.y() - pos.x()});
    
    pos.ry()++;
    
    if (err < 0) {
      err += 2 * pos.y() + 1;
    } else {
      pos.rx()--;
      err += 2 * (pos.y() - pos.x()) + 1;
    }
  }
  
  return drawn;
}

template <typename Pixel>
bool midpointThickCircle(
  QImage &image,
  Pixel color,
  QPoint center,
  int innerRadius,
  int outerRadius,
  CircleShape shape
) {
  assert(0 <= innerRadius);
  assert(innerRadius <= outerRadius);

  Surface surface = makeSurface<Pixel>(image);
  int innerX = innerRadius;
  int outerX = outerRadius;
  int posY = 0;
  int innerErr = 1 - innerRadius;
  int outerErr = 1 - outerRadius;
  const int extraX = centerOffsetX(shape);
  const int extraY = centerOffsetY(shape);
  bool drawn = false;
  
  while (outerX >= posY) {
    drawn |= surface.horiLineClip(color, {center.x() + innerX + extraX, center.y() + posY + extraY},   center.x() + outerX + extraX); // right down
    drawn |= surface.vertLineClip(color, {center.x() + posY + extraX,   center.y() + innerX + extraY}, center.y() + outerX + extraY); // right down
    drawn |= surface.horiLineClip(color, {center.x() - outerX,          center.y() + posY + extraY},   center.x() - innerX);          //       down
    drawn |= surface.vertLineClip(color, {center.x() - posY,            center.y() + innerX + extraY}, center.y() + outerX + extraY); //       down
    
    drawn |= surface.horiLineClip(color, {center.x() - outerX,          center.y() - posY},   center.x() - innerX);                   //
    drawn |= surface.vertLineClip(color, {center.x() - posY,            center.y() - outerX}, center.y() - innerX);                   //
    drawn |= surface.horiLineClip(color, {center.x() + innerX + extraX, center.y() - posY},   center.x() + outerX + extraX);          // right
    drawn |= surface.vertLineClip(color, {center.x() + posY + extraX,   center.y() - outerX}, center.y() - innerX);                   // right
    
    posY++;
    
    if (outerErr < 0) {
      outerErr += 2 * posY + 1;
    } else {
      outerX--;
      outerErr += 2 * (posY - outerX) + 1;
    }
    
    if (posY > innerRadius) {
      innerX = posY;
    } else {
      if (innerErr < 0) {
        innerErr += 2 * posY + 1;
      } else {
        innerX--;
        innerErr += 2 * (posY - innerX) + 1;
      }
    }
  }
  
  return drawn;
}

}

bool drawStrokedCircle(QImage &img, const QRgb color, const QPoint center, const int radius, const int thickness, const CircleShape shape) {
  assert(thickness > 0);
  if (img.depth() == 8) {
    if (thickness == 1) {
      return midpointCircle<uint8_t>(img, color, center, radius, shape);
    } else {
      return midpointThickCircle<uint8_t>(img, color, center, std::max(radius - thickness + 1, 0), radius, shape);
    }
  } else if (img.depth() == 32) {
    if (thickness == 1) {
      return midpointCircle<uint32_t>(img, color, center, radius, shape);
    } else {
      return midpointThickCircle<uint32_t>(img, color, center, std::max(radius - thickness + 1, 0), radius, shape);
    }
  } else {
    Q_UNREACHABLE();
  }
}

bool drawFilledRect(QImage &img, const QRgb color, const QRect rect) {
  if (img.depth() == 8) {
    return makeSurface<uint8_t>(img).fillRectClip(color, rect);
  } else if (img.depth() == 32) {
    return makeSurface<uint32_t>(img).fillRectClip(color, rect);
  } else {
    Q_UNREACHABLE();
  }
}

namespace {

template <typename Pixel>
bool strokedRect(QImage &image, const Pixel color, const QRect rect, const int thickness) {
  Surface surface = makeSurface<Pixel>(image);
  if (rect.width() <= thickness * 2 || rect.height() <= thickness * 2) {
    return surface.fillRectClip(color, rect);
  }
  const QRect sideRects[] = {
    { // top
      QPoint{rect.left(), rect.top()},
      QPoint{rect.right(), rect.top() + thickness - 1}
    },
    { // left
      QPoint{rect.left(), rect.top() + thickness},
      QPoint{rect.left() + thickness - 1, rect.bottom() - thickness}
    },
    { // right
      QPoint{rect.right() - thickness + 1, rect.top() + thickness},
      QPoint{rect.right(), rect.bottom() - thickness}
    },
    { // bottom
      QPoint{rect.left(), rect.bottom() - thickness + 1},
      QPoint{rect.right(), rect.bottom()}
    }
  };
  bool drawn = false;
  for (const QRect &sideRect : sideRects) {
    drawn |= surface.fillRectClip(color, sideRect);
  }
  return drawn;
}

}

bool drawStrokedRect(QImage &img, const QRgb color, const QRect rect, const int thickness) {
  assert(thickness > 0);
  if (!img.rect().intersects(rect)) return false;
  if (img.depth() == 8) {
    return strokedRect<uint8_t>(img, color, rect, thickness);
  } else if (img.depth() == 32) {
    return strokedRect<uint32_t>(img, color, rect, thickness);
  } else {
    Q_UNREACHABLE();
  }
}

namespace {

std::pair<int, int> signdiff(const int a, const int b) {
  if (a < b) {
    return {1, b - a};
  } else {
    return {-1, a - b};
  }
}

template <typename SetPixel>
bool midpointLine(QPoint p1, const QPoint p2, SetPixel &&setPixel) {
  const auto [sx, dx] = signdiff(p1.x(), p2.x());
  auto [sy, dy] = signdiff(p1.y(), p2.y());
  dy = -dy;
  int err = dx + dy;
  bool drawn = false;
  
  while (true) {
    drawn |= setPixel(p1);
    const int err2 = 2 * err;
    if (err2 >= dy) {
      if (p1.x() == p2.x()) break;
      err += dy;
      p1.rx() += sx;
    }
    if (err2 <= dx) {
      if (p1.y() == p2.y()) break;
      err += dx;
      p1.ry() += sy;
    }
  }
  
  return drawn;
}

template <typename Pixel>
bool midpointLine(QImage &img, const Pixel col, const QPoint p1, const QPoint p2) {
  Surface surface = makeSurface<Pixel>(img);
  return midpointLine(p1, p2, [surface, col](const QPoint pos) mutable {
    return surface.setPixelClip(col, pos);
  });
}

// @TODO this is suboptimial but seems to be fast enough
template <typename Pixel>
bool midpointThickLine(QImage &img, const Pixel col, const QPoint p1, const QPoint p2, const int thickness) {
  midpointFilledCircle(img, col, p1, thickness, CircleShape::c1x1);
  return midpointLine(p1, p2, [&img, col, thickness](const QPoint pos) {
    return midpointThickCircle(img, col, pos, thickness - 1, thickness, CircleShape::c1x1);
  });
}

}

bool drawLine(QImage &img, const QRgb color, const QLine line, const int thickness) {
  assert(thickness > 0);
  if (img.depth() == 8) {
    if (thickness == 1) {
      return midpointLine<uint8_t>(img, color, line.p1(), line.p2());
    } else {
      return midpointThickLine<uint8_t>(img, color, line.p1(), line.p2(), thickness);
    }
  } else if (img.depth() == 32) {
    if (thickness == 1) {
      return midpointLine<uint32_t>(img, color, line.p1(), line.p2());
    } else {
      return midpointThickLine<uint32_t>(img, color, line.p1(), line.p2(), thickness);
    }
  } else {
    Q_UNREACHABLE();
  }
}

bool drawFilledPolygon(
  QImage &img,
  const QRgb color,
  const Polygon &poly,
  const QPoint offset
) {
  std::vector<QPoint> shiftedPoly;
  shiftedPoly.reserve(poly.size());
  for (const QPoint vertex : poly) {
    shiftedPoly.push_back(vertex + offset);
  }
  QPainter painter{&img};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setBrush(QColor::fromRgba(color));
  painter.setPen(QColor::fromRgba(color));
  painter.drawPolygon(shiftedPoly.data(), static_cast<int>(shiftedPoly.size()));
  return true;
}
