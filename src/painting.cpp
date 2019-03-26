//
//  painting.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 21/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "painting.hpp"

#include "utils.hpp"
#include <QtGui/qpainter.h>

namespace {

int centerWidth(const CircleShape shape) {
  return (shape == CircleShape::c2x1 || shape == CircleShape::c2x2) ? 2 : 1;
}

int centerHeight(const CircleShape shape) {
  return (shape == CircleShape::c1x2 || shape == CircleShape::c2x2) ? 2 : 1;
}

}

QRect circleToRect(const QPoint center, const int radius, const CircleShape shape) {
  return QRect{
    center.x() - radius,
    center.y() - radius,
    radius * 2 + centerWidth(shape),
    radius * 2 + centerHeight(shape)
  };
}

QRect centerToRect(const QPoint center, const CircleShape shape) {
  return circleToRect(center, 0, shape);
}

namespace {

QRect adjustStrokedEllipse(const QRect rect, const int thickness) {
  return QRect{
    rect.left() + thickness / 2,
    rect.top() + thickness / 2,
    rect.width() - thickness,
    rect.height() - thickness
  };
}

const QPen round_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin
};
const QPen square_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin
};

QPen makePen(const QPen &base, const QRgb color, const int width) {
  QPen pen = base;
  pen.setColor(toColor(color));
  pen.setWidth(width);
  return pen;
}

void preparePainter(QPainter &painter) {
  painter.setCompositionMode(QPainter::CompositionMode_Source);
}

template <typename Pixel>
Pixel *pixelAddr(
  uchar *const bits,
  const int bbl,
  const QPoint pos
) noexcept {
  return reinterpret_cast<Pixel *>(bits + pos.y() * bbl + pos.x() * sizeof(Pixel));
}

template <typename Pixel>
bool drawSquarePoint(QImage &img, const QRgb color, const QPoint pos) {
  *pixelAddr<Pixel>(img.bits(), img.bytesPerLine(), pos) = static_cast<Pixel>(color);
  return true;
}

}

bool drawSquarePoint(QImage &img, const QRgb color, const QPoint pos) {
  if (!img.rect().contains(pos)) return false;
  if (img.depth() == 8) {
    return drawSquarePoint<uint8_t>(img, color, pos);
  } else if (img.depth() == 32) {
    return drawSquarePoint<uint32_t>(img, color, pos);
  } else {
    Q_UNREACHABLE();
  }
}

bool drawRoundPoint(QImage &img, const QRgb color, const QPoint pos, const int thickness) {
  QPainter painter{&img};
  preparePainter(painter);
  painter.setPen(makePen(round_pen, color, thickness));
  painter.drawPoint(pos);
  return true;
}

namespace {

template <typename Pixel>
class PixelGetter {
public:
  PixelGetter(QImage &img, const Pixel startColor, const Pixel toolColor)
    : bits{img.bits()},
      bbl{img.bytesPerLine()},
      startColor{startColor},
      toolColor{toolColor} {}
  
  bool filled(const QPoint pos) const noexcept {
    return *pixelAddr<Pixel>(bits, bbl, pos) != startColor;
  }
  void fill(const QPoint pos) noexcept {
    *pixelAddr<Pixel>(bits, bbl, pos) = toolColor;
  }

private:
  uchar *const bits;
  const int bbl;
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
void startFloodFill(PixelGetter<Pixel>, QPoint, QSize);

template <typename Pixel>
void floodFillImpl(PixelGetter<Pixel> px, QPoint pos, const QSize size) {
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
        if (pos.y() != 0 && !px.filled(up(pos))) startFloodFill(px, up(pos), size);
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
        if (!px.filled(start)) floodFillImpl(px, start, size);
      }
    } else if (rowLength > lastRowLength && pos.y() != 0) {
      QPoint above = up({pos.x() + lastRowLength, pos.y()});
      while (++above.rx() < start.x()) {
        if (!px.filled(above)) startFloodFill(px, above, size);
      }
    }
    lastRowLength = rowLength;
    pos = down(pos);
  } while (lastRowLength != 0 && pos.y() < size.height());
}

template <typename Pixel>
void startFloodFill(PixelGetter<Pixel> px, QPoint pos, const QSize size) {
  while (true) {
    const QPoint startPos = pos;
    while (pos.y() != 0 && !px.filled(up(pos))) pos = up(pos);
    while (pos.x() != 0 && !px.filled(left(pos))) pos = left(pos);
    if (pos == startPos) break;
  }
  floodFillImpl(px, pos, size);
}

template <typename Pixel>
bool floodFill(QImage &img, const QPoint startPos, const QRgb color) {
  img.detach();
  const Pixel toolColor = static_cast<Pixel>(color);
  const Pixel startColor = *pixelAddr<Pixel>(img.bits(), img.bytesPerLine(), startPos);
  if (startColor == toolColor) return false;
  PixelGetter<Pixel> px{img, startColor, toolColor};
  startFloodFill(px, startPos, img.size());
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

bool drawFilledEllipse(QImage &img, const QRgb color, const QRect ellipse) {
  assert(ellipse.isValid());
  if (!img.rect().intersects(ellipse)) return false;
  QPainter painter{&img};
  preparePainter(painter);
  painter.setBrush(toColor(color));
  painter.setPen(makePen(round_pen, color, 1));
  painter.drawEllipse(adjustStrokedEllipse(ellipse, 1));
  return true;
}

bool drawStrokedEllipse(QImage &img, const QRgb color, const QRect ellipse) {
  assert(ellipse.isValid());
  if (!img.rect().intersects(ellipse)) return false;
  QPainter painter{&img};
  preparePainter(painter);
  painter.setPen(makePen(round_pen, color, 1));
  painter.drawEllipse(adjustStrokedEllipse(ellipse, 1));
  return true;
}

namespace {

template <typename Pixel>
void fillScanLine(Pixel *row, const uintptr_t size, const Pixel color) noexcept {
  Pixel *const rowEnd = row + size;
  while (row != rowEnd) {
    *row++ = color;
  }
}

template <>
void fillScanLine(uint8_t *const row, const uintptr_t size, const uint8_t color) noexcept {
  std::memset(row, color, size);
}

template <typename Pixel>
void fillRect(QImage &img, const QRgb color, const QPoint topLeft, const QSize size) {
  img.detach();
  const Pixel toolColor = static_cast<Pixel>(color);
  const uintptr_t ppl = img.bytesPerLine() / sizeof(Pixel);
  Pixel *row = reinterpret_cast<Pixel *>(img.bits()) + topLeft.y() * ppl + topLeft.x();
  Pixel *const endRow = row + size.height() * ppl;
  const uintptr_t width = size.width();
  
  while (row != endRow) {
    fillScanLine(row, width, toolColor);
    row += ppl;
  }
}

}

bool drawFilledRect(QImage &img, const QRgb color, const QRect rect) {
  const QRect clipped = img.rect().intersected(rect);
  if (clipped.isEmpty()) return false;
  
  if (img.depth() == 8) {
    fillRect<uint8_t>(img, color, clipped.topLeft(), clipped.size());
  } else if (img.depth() == 32) {
    fillRect<uint32_t>(img, color, clipped.topLeft(), clipped.size());
  } else {
    Q_UNREACHABLE();
  }
  
  return true;
}

bool drawStrokedRect(QImage &img, const QRgb color, const QRect rect) {
  if (!img.rect().intersects(rect)) return false;
  bool drawn = false;
  drawn |= drawLine(img, color, {rect.topLeft(), rect.topRight()});
  drawn |= drawLine(img, color, {rect.topRight(), rect.bottomRight()});
  drawn |= drawLine(img, color, {rect.bottomLeft(), rect.bottomRight()});
  drawn |= drawLine(img, color, {rect.topLeft(), rect.bottomLeft()});
  return drawn;
}

namespace {

template <typename Pixel>
void setPixel(QImage &img, const QPoint pos, const Pixel color) {
  // @TODO clip the line
  // we can increment and decrement the pointer instead of using this function
  if (img.rect().contains(pos)) {
    const uintptr_t ppl = img.bytesPerLine() / sizeof(Pixel);
    Pixel *const px = reinterpret_cast<Pixel *>(img.bits()) + pos.y() * ppl + pos.x();
    *px = color;
  }
}

template <typename Pixel>
void lineLilPos(QImage &img, const Pixel col, const QPoint p1, const QPoint p2) {
  const int dx = p2.x() - p1.x();
  const int dy = p2.y() - p1.y();
  const int incE = 2 * dy;
  const int incSE = 2 * (dy - dx);
  int err = 2 * dy - dx;
  QPoint pos = p1;
  
  setPixel(img, pos, col);
  while (pos.x() < p2.x()) {
    if (err < 0) {
      err += incE;
    } else {
      err += incSE;
      ++pos.ry();
    }
    ++pos.rx();
    setPixel(img, pos, col);
  }
}

template <typename Pixel>
void lineBigPos(QImage &img, const Pixel col, const QPoint p1, const QPoint p2) {
  const int dx = p2.x() - p1.x();
  const int dy = p2.y() - p1.y();
  const int incS = 2 * dx;
  const int incSE = 2 * (dx - dy);
  int err = 2 * dx - dy;
  QPoint pos = p1;
  
  setPixel(img, pos, col);
  while (pos.y() < p2.y()) {
    if (err < 0) {
      err += incS;
    } else {
      err += incSE;
      ++pos.rx();
    }
    ++pos.ry();
    setPixel(img, pos, col);
  }
}

template <typename Pixel>
void lineLilNeg(QImage &img, const Pixel col, const QPoint p1, const QPoint p2) {
  const int dx = p2.x() - p1.x();
  const int dy = p1.y() - p2.y();
  const int incE = 2 * dy;
  const int incNE = 2 * (dy - dx);
  int err = 2 * dy - dx;
  QPoint pos = p1;
  
  setPixel(img, pos, col);
  while (pos.x() < p2.x()) {
    if (err < 0) {
      err += incE;
    } else {
      err += incNE;
      --pos.ry();
    }
    ++pos.rx();
    setPixel(img, pos, col);
  }
}

template <typename Pixel>
void lineBigNeg(QImage &img, const Pixel col, const QPoint p1, const QPoint p2) {
  const int dx = p2.x() - p1.x();
  const int dy = p1.y() - p2.y();
  const int incN = 2 * dx;
  const int incNE = 2 * (dx - dy);
  int err = 2 * dx - dy;
  QPoint pos = p1;
  
  setPixel(img, pos, col);
  while (pos.y() > p2.y()) {
    if (err < 0) {
      err += incN;
    } else {
      err += incNE;
      ++pos.rx();
    }
    --pos.ry();
    setPixel(img, pos, col);
  }
}

template <typename Pixel>
void midpointLine(QImage &img, const Pixel col, QPoint p1, QPoint p2) {
  img.detach();
  
  if (p2.x() < p1.x()) {
    std::swap(p1, p2);
  }
  
  const int dx = p2.x() - p1.x();
  const int dy = p2.y() - p1.y();
  
  if (0 < dy && dy <= dx) {
    return lineLilPos(img, col, p1, p2);
  } else if (dx < dy) {
    return lineBigPos(img, col, p1, p2);
  } else if (-dy <= dx) {
    return lineLilNeg(img, col, p1, p2);
  } else {
    return lineBigNeg(img, col, p1, p2);
  }
}

}

bool drawLine(QImage &img, const QRgb color, const QLine line) {
  if (img.depth() == 8) {
    midpointLine<uint8_t>(img, color, line.p1(), line.p2());
  } else if (img.depth() == 32) {
    midpointLine<uint32_t>(img, color, line.p1(), line.p2());
  } else {
    Q_UNREACHABLE();
  }
  return true;
}

bool drawRoundLine(QImage &img, const QRgb color, const QLine line, const int thickness) {
  QPainter painter{&img};
  preparePainter(painter);
  painter.setPen(makePen(round_pen, color, thickness));
  painter.drawLine(line);
  return true;
}

bool drawFilledPolygon(
  QImage &img,
  const QRgb color,
  const std::vector<QPoint> &poly,
  const QPoint offset
) {
  std::vector<QPoint> shiftedPoly;
  shiftedPoly.reserve(poly.size());
  for (const QPoint vertex : poly) {
    shiftedPoly.push_back(vertex + offset);
  }
  QPainter painter{&img};
  preparePainter(painter);
  painter.setBrush(toColor(color));
  painter.setPen(makePen(round_pen, color, 1));
  painter.drawPolygon(shiftedPoly.data(), static_cast<int>(shiftedPoly.size()));
  return true;
}
