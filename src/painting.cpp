//
//  painting.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 21/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "painting.hpp"

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

QRect adjustStrokedRect(const QRect rect, const int thickness) {
  return QRect{
    rect.left() + thickness / 2,
    rect.top() + thickness / 2,
    rect.width() - thickness,
    rect.height() - thickness
  };
}

QRect adjustStrokedEllipse(const QRect rect, const int thickness) {
  return adjustStrokedRect(rect, thickness);
}

namespace {

QColor toColor(const QRgb rgba) {
  // the QRgb constructor sets alpha to 255 for some reason
  return QColor{qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba)};
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
  painter.setRenderHint(QPainter::Antialiasing, false);
}

}

bool drawSquarePoint(QImage &img, const QRgb color, const QPoint pos) {
  QPainter painter{&img};
  preparePainter(painter);
  painter.setPen(makePen(square_pen, color, 1));
  painter.drawPoint(pos);
  return true;
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
Pixel *pixelAddr(uchar *bits, const int bbl, const QPoint pos) noexcept {
  return reinterpret_cast<Pixel *>(bits + bbl * pos.y()) + pos.x();
}

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

bool drawFilledEllipse(
  QImage &img,
  const QRgb color,
  const QRect ellipse
) {
  assert(!ellipse.size().isEmpty());
  if (!img.rect().intersects(ellipse)) return false;
  QPainter painter{&img};
  preparePainter(painter);
  painter.setBrush(toColor(color));
  painter.setPen(makePen(round_pen, color, 1));
  painter.drawEllipse(adjustStrokedEllipse(ellipse, 1));
  return true;
}

bool drawStrokedEllipse(
  QImage &img,
  const QRgb color,
  const QRect ellipse
) {
  assert(!ellipse.size().isEmpty());
  if (!img.rect().intersects(ellipse)) return false;
  QPainter painter{&img};
  preparePainter(painter);
  painter.setPen(makePen(round_pen, color, 1));
  painter.drawEllipse(adjustStrokedEllipse(ellipse, 1));
  return true;
}

namespace {

template <typename Pixel>
void fillScanLine(Pixel *row, const int size, const Pixel color) {
  Pixel *const rowEnd = row + size;
  for (; row != rowEnd; ++row) {
    *row = color;
  }
}

template <>
void fillScanLine(uint8_t *row, const int size, const uint8_t color) {
  std::memset(row, color, size);
}

template <typename Pixel>
void fillRect(QImage &img, const QRgb color, const QPoint topLeft, const QSize size) {
  img.detach();
  const Pixel toolColor = static_cast<Pixel>(color);
  uchar *const bits = img.bits();
  const int bbl = img.bytesPerLine();
  int y = topLeft.y();
  const int endY = y + size.height();
  for (; y != endY; ++y) {
    Pixel *row = pixelAddr<Pixel>(bits, bbl, {topLeft.x(), y});
    fillScanLine(row, size.width(), toolColor);
  }
}

}

bool drawFilledRect(QImage &img, const QRgb color, const QRect rect) {
  assert(rect.width() >= 0);
  assert(rect.height() >= 0);
  const QRect clipped = img.rect().intersected(rect);
  if (clipped.isEmpty()) return false;
  
  if (img.depth() == 8) {
    fillRect<uint8_t>(img, color, rect.topLeft(), rect.size());
  } else if (img.depth() == 32) {
    fillRect<uint32_t>(img, color, rect.topLeft(), rect.size());
  } else {
    Q_UNREACHABLE();
  }
  
  return true;
}

bool drawStrokedRect(
  QImage &img,
  const QRgb color,
  const QRect rect
) {
  assert(rect.width() >= 0);
  assert(rect.height() >= 0);
  if (!img.rect().intersects(rect)) return false;
  
  QPainter painter{&img};
  preparePainter(painter);
  painter.setPen(makePen(square_pen, color, 1));
  painter.drawRect(adjustStrokedRect(rect, 1));
  
  return true;
}

bool drawLine(
  QImage &img,
  const QRgb color,
  const QLine line
) {
  QPainter painter{&img};
  preparePainter(painter);
  painter.setPen(makePen(square_pen, color, 1));
  painter.drawLine(line);
  return true;
}

bool drawRoundLine(
  QImage &img,
  const QRgb color,
  const QLine line,
  const int thickness
) {
  QPainter painter{&img};
  preparePainter(painter);
  painter.setPen(makePen(round_pen, color, thickness));
  painter.drawLine(line);
  return true;
}
