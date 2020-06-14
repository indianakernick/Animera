//
//  painting.cpp
//  Animera
//
//  Created by Indiana Kernick on 21/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
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
  const PixelVar color,
  const QPoint pos,
  const gfx::CircleShape shape
) {
  if (img.isNull()) return false;
  return visitSurfaces(img, color, [=](auto surface, auto color) {
    return gfx::drawFilledRect(surface, color, gfx::centerRect(convert(pos), shape));
  });
}

bool drawRoundPoint(
  QImage &img,
  const PixelVar color,
  const QPoint pos,
  const int radius,
  const gfx::CircleShape shape
) {
  assert(radius >= 0);
  if (img.isNull()) return false;
  if (radius == 0) {
    return drawSquarePoint(img, color, pos, shape);
  } else {
    return drawFilledCircle(img, color, pos, radius, shape);
  }
}

bool drawFloodFill(QImage &img, const PixelVar color, QPoint pos, QRect rect) {
  assert(!rect.isEmpty());
  if (img.isNull()) return false;
  if (!img.rect().contains(pos)) return false;
  pos -= rect.topLeft();
  return visitSurfaces(img, color, [=](auto surface, auto color) {
    return gfx::drawFloodFill(surface.view(convert(rect)), color, convert(pos));
  });
}

bool drawFilledCircle(
  QImage &img,
  const PixelVar color,
  const QPoint center,
  const int radius,
  const gfx::CircleShape shape
) {
  if (img.isNull()) return false;
  return visitSurfaces(img, color, [=](auto surface, auto color) {
    return gfx::drawFilledCircle(surface, color, convert(center), radius, shape);
  });
}

bool drawStrokedCircle(
  QImage &img,
  const PixelVar color,
  const QPoint center,
  const int radius,
  const int thickness,
  const gfx::CircleShape shape
) {
  assert(thickness > 0);
  if (img.isNull()) return false;
  return visitSurfaces(img, color, [=](auto surface, auto color) {
    return gfx::drawStrokedCircle(surface, color, convert(center), radius, radius - thickness + 1, shape);
  });
}

bool drawFilledRect(QImage &img, const PixelVar color, const QRect rect) {
  if (img.isNull()) return false;
  return visitSurfaces(img, color, [=](auto surface, auto color) {
    return gfx::drawFilledRect(surface, color, convert(rect));
  });
}

bool drawStrokedRect(
  QImage &img,
  const PixelVar color,
  const QRect rect,
  const int thickness
) {
  assert(thickness > 0);
  if (img.isNull()) return false;
  if (!img.rect().intersects(rect)) return false;
  return visitSurfaces(img, color, [=](auto surface, auto color) {
    const QRect inner = rect.marginsRemoved({thickness, thickness, thickness, thickness});
    return gfx::drawStrokedRect(surface, color, convert(rect), convert(inner));
  });
}

namespace {

// TODO: Should we use the canvas format instead of the pixel type?

template <typename Pixel>
struct PixelFormat;

template <>
struct PixelFormat<PixelIndex> {
  // TODO: Interpolating indexed colors. Is this the best we can do?
  using type = gfx::Y;
};

template <>
struct PixelFormat<PixelGray> {
  using type = FmtGray;
};

template <>
struct PixelFormat<PixelRgba> {
  using type = FmtRgba;
};

std::uint8_t interpolate(const std::uint8_t a, const std::uint8_t b, const int index, const int size) {
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
  const PixelVar left,
  const PixelVar right,
  const QRect rect
) {
  if (img.isNull()) return false;
  return visitSurfaces(img, left, right, [=](auto surface, auto left, auto right) {
    using Pixel = typename decltype(surface)::Pixel;
    auto func = makeInterpolator<Pixel>(left, right);
    return gfx::drawHoriGradient(surface, convert(rect), func);
  });
}

bool drawVertGradient(
  QImage &img,
  const PixelVar top,
  const PixelVar bottom,
  const QRect rect
) {
  if (img.isNull()) return false;
  return visitSurfaces(img, top, bottom, [=](auto surface, auto top, auto bottom) {
    using Pixel = typename decltype(surface)::Pixel;
    auto func = makeInterpolator<Pixel>(top, bottom);
    return gfx::drawVertGradient(surface, convert(rect), func);
  });
}

bool drawLine(QImage &img, const PixelVar color, const QLine line, const int radius) {
  if (img.isNull()) return false;
  return visitSurfaces(img, color, [=](auto surface, auto color) {
    return gfx::drawLine(surface, color, convert(line.p1()), convert(line.p2()), radius);
  });
}

bool drawFilledPolygon(
  QImage &img,
  const QRgb color,
  const std::vector<QPoint> &poly
) {
  if (img.isNull()) return false;
  // TODO: avoid using QPainter
  QPainter painter{&img};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setBrush(QColor::fromRgba(color));
  painter.setPen(QColor::fromRgba(color));
  painter.drawPolygon(poly.data(), static_cast<int>(poly.size()));
  return true;
}
