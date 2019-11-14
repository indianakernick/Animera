//
//  painting.cpp
//  Animera
//
//  Created by Indi Kernick on 21/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "painting.hpp"

#include "math.hpp"
#include "geometry.hpp"
#include <QtGui/qpainter.h>
#include <Graphics/draw.hpp>
#include <Graphics/format.hpp>
#include "surface factory.hpp"
#include "graphics convert.hpp"

bool drawSquarePoint(
  QImage &img,
  const QRgb color,
  const QPoint pos,
  const gfx::CircleShape shape
) {
  return visitSurface(img, [color, pos, shape](auto surface) {
    return gfx::drawFilledRect(surface, color, gfx::centerRect(convert(pos), shape));
  });
}

bool drawRoundPoint(
  QImage &img,
  const QRgb color,
  const QPoint pos,
  const int radius,
  const gfx::CircleShape shape
) {
  assert(radius >= 0);
  if (radius == 0) {
    return drawSquarePoint(img, color, pos, shape);
  } else {
    return drawFilledCircle(img, color, pos, radius, shape);
  }
}

bool drawFloodFill(QImage &img, const QRgb color, QPoint pos, QRect rect) {
  assert(!rect.isEmpty());
  if (!img.rect().contains(pos)) return false;
  pos -= rect.topLeft();
  return visitSurface(img, [pos, color, rect](auto surface) {
    return gfx::drawFloodFill(surface.view(convert(rect)), color, convert(pos));
  });
}

bool drawFilledCircle(
  QImage &img,
  const QRgb color,
  const QPoint center,
  const int radius,
  const gfx::CircleShape shape
) {
  return visitSurface(img, [center, radius, shape, color](auto surface) {
    return gfx::drawFilledCircle(surface, color, convert(center), radius, shape);
  });
}

bool drawStrokedCircle(
  QImage &img,
  const QRgb color,
  const QPoint center,
  const int radius,
  const int thickness,
  const gfx::CircleShape shape
) {
  assert(thickness > 0);
  return visitSurface(img, [center, radius, thickness, shape, color](auto surface) {
    return gfx::drawStrokedCircle(surface, color, convert(center), radius, radius - thickness + 1, shape);
  });
}

bool drawFilledRect(QImage &img, const QRgb color, const QRect rect) {
  return visitSurface(img, [rect, color](auto surface) {
    return gfx::drawFilledRect(surface, color, convert(rect));
  });
}

bool drawStrokedRect(
  QImage &img,
  const QRgb color,
  const QRect rect,
  const int thickness
) {
  assert(thickness > 0);
  if (!img.rect().intersects(rect)) return false;
  return visitSurface(img, [rect, thickness, color](auto surface) {
    const QRect inner = rect.marginsRemoved({thickness, thickness, thickness, thickness});
    return gfx::drawStrokedRect(surface, color, convert(rect), convert(inner));
  });
}

namespace {

// TODO: Should we use the canvas format instead of the pixel type?

template <typename Pixel>
struct PixelFormat;

template <>
struct PixelFormat<uint8_t> {
  // TODO: Interpolating indexed colors. Is this the best we can do?
  using type = gfx::Y;
};

template <>
struct PixelFormat<uint16_t> {
  using type = gfx::YA;
};

template <>
struct PixelFormat<uint32_t> {
  using type = gfx::ARGB;
};

uint8_t interpolate(const uint8_t a, const uint8_t b, const int index, const int size) {
  // TODO: remove the if
  if (a > b) {
    return scale(size - index, 0, size, b, a);
  } else {
    return scale(index, 0, size, a, b);
  }
}

template <typename Pixel>
auto makeInterpolator(const Pixel first, const Pixel second) {
  using Format = typename PixelFormat<Pixel>::type;
  return [first, second](const int index, const int size) noexcept {
    const gfx::Color firstColor = Format::color(first);
    const gfx::Color secondColor = Format::color(second);
    const gfx::Color middleColor = {
      interpolate(firstColor.r, secondColor.r, index, size),
      interpolate(firstColor.g, secondColor.g, index, size),
      interpolate(firstColor.b, secondColor.b, index, size),
      interpolate(firstColor.a, secondColor.a, index, size),
    };
    return Format::pixel(middleColor);
  };
}

}

bool drawHoriGradient(
  QImage &img,
  const QRgb left,
  const QRgb right,
  const QRect rect
) {
  return visitSurface(img, [left, right, rect](auto surface) {
    using Pixel = typename decltype(surface)::Pixel;
    auto func = makeInterpolator<Pixel>(left, right);
    return gfx::drawHoriGradient(surface, convert(rect), func);
  });
}

bool drawVertGradient(
  QImage &img,
  const QRgb top,
  const QRgb bottom,
  const QRect rect
) {
  return visitSurface(img, [top, bottom, rect](auto surface) {
    using Pixel = typename decltype(surface)::Pixel;
    auto func = makeInterpolator<Pixel>(top, bottom);
    return gfx::drawVertGradient(surface, convert(rect), func);
  });
}

bool drawLine(QImage &img, const QRgb color, const QLine line, const int radius) {
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
