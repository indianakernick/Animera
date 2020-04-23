//
//  composite.cpp
//  Animera
//
//  Created by Indiana Kernick on 18/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "composite.hpp"

#include <limits>
#include "config.hpp"
#include "scope time.hpp"
#include <Graphics/copy.hpp>
#include <Graphics/fill.hpp>
#include <Graphics/mask.hpp>
#include "surface factory.hpp"
#include "graphics convert.hpp"
#include <Graphics/porter duff.hpp>

namespace {

template <typename Format>
using Surface = gfx::Surface<gfx::Pixel<Format>>;

template <typename DstFormat, typename SrcFormat>
void copy(Surface<DstFormat> dst, const Cel &cel, const QPoint dstPos, SrcFormat srcFmt) {
  gfx::Surface src = makeCSurface<gfx::Pixel<SrcFormat>>(cel.img);
  const gfx::Point pos = convert(cel.pos - dstPos);
  if constexpr (std::is_same_v<DstFormat, SrcFormat>) {
    gfx::copyRegion(dst, src, pos);
  } else {
    gfx::eachRegion(dst, src, pos, [srcFmt](auto &dst, const auto src) {
      dst = DstFormat::pixel(srcFmt.color(src));
    });
  }
}

template <typename DstFormat, typename SrcFormat>
void composite(Surface<DstFormat> dst, const Cel &cel, const QPoint dstPos, SrcFormat srcFmt) {
  gfx::porterDuffRegion(
    gfx::mode_src_over,
    dst,
    makeCSurface<gfx::Pixel<SrcFormat>>(cel.img),
    DstFormat{},
    srcFmt,
    convert(cel.pos - dstPos)
  );
}

template <typename DstFormat, typename SrcFormat>
void compositeFrame(Surface<DstFormat> dst, const Frame &frame, const QPoint dstPos, SrcFormat srcFmt) {
  // Layer 0 is on top of layer 1
  const QRect dstRect = {dstPos, convert(dst.size())};
  for (std::size_t c = frame.size() - 1; c != ~std::size_t{}; --c) {
    const Cel &cel = *frame[c];
    const QRect celRect = dstRect.intersected(cel.rect());
    if (celRect.isEmpty()) continue;
    bool overlap = false;
    for (std::size_t d = c + 1; d != frame.size(); ++d) {
      if (celRect.intersects(frame[d]->rect())) {
        overlap = true;
        break;
      }
    }
    if (overlap) {
      composite<DstFormat>(dst, cel, dstPos, srcFmt);
    } else {
      copy<DstFormat>(dst, cel, dstPos, srcFmt);
    }
  }
}

}

template <typename PxFmt>
void compositeFrame(
  QImage &dst,
  PaletteCSpan palette,
  const Frame &frame,
  const Format format,
  QRect rect
) {
  SCOPE_TIME("compositeFrame");
  
  rect = rect.intersected(dst.rect());
  if (rect.isEmpty()) return;
  auto dstSurface = makeSurface<gfx::Pixel<PxFmt>>(dst).view(convert(rect));
  
  gfx::fill(dstSurface);
  
  switch (format) {
    case Format::rgba:
      compositeFrame<PxFmt>(dstSurface, frame, rect.topLeft(), gfx::ARGB{});
      break;
    case Format::index:
      compositeFrame<PxFmt>(dstSurface, frame, rect.topLeft(), gfx::I<>{palette.data()});
      break;
    case Format::gray:
      compositeFrame<PxFmt>(dstSurface, frame, rect.topLeft(), gfx::YA{});
      break;
    default: Q_UNREACHABLE();
  }
}

template void compositeFrame<gfx::ARGB>(QImage &, PaletteCSpan, const Frame &, Format, QRect);
template void compositeFrame<gfx::YA>(QImage &, PaletteCSpan, const Frame &, Format, QRect);

void compositeOverlay(QImage &drawing, const QImage &overlay) {
  gfx::porterDuff(
    gfx::mode_src_over,
    makeSurface<PixelRgba>(drawing),
    makeSurface<PixelRgba>(overlay),
    gfx::ARGB{},
    gfx::ARGB{}
  );
}

void blitImage(QImage &dst, const QImage &src, const QPoint pos) {
  visitSurfaces(dst, src, [pos](auto dst, auto src) {
    gfx::copyRegion(dst, src, convert(pos));
  });
}

void blitMaskImage(QImage &dst, const QImage &mask, const QImage &src, const QPoint pos) {
  assert(mask.size() == src.size());
  visitSurfaces(dst, src, [&mask, pos](auto dst, auto src) {
    gfx::maskCopyRegion(
      dst,
      src,
      makeSurface<PixelMask>(mask),
      convert(pos),
      convert(pos)
    );
  });
}

void fillMaskImage(QImage &dst, const QImage &mask, const QRgb color, const QPoint pos) {
  visitSurface(dst, [&mask, pos, color](auto dst) {
    gfx::maskFillRegion(dst, makeCSurface<PixelMask>(mask), color, convert(pos));
  });
}

namespace {

PixelRgba rgbaToOverlayPx(const PixelRgba pixel) {
  const gfx::Color color = gfx::ARGB::color(pixel);
  const std::uint8_t gray = gfx::gray(color);
  const std::uint8_t alpha = scaleOverlayAlpha(color.a);
  return qRgba(gray, gray, gray, alpha);
}

PixelRgba paletteToOverlayPx(const PaletteCSpan palette, const PixelIndex pixel) {
  return rgbaToOverlayPx(palette[pixel]);
}

PixelRgba grayToOverlayPx(const PixelGray pixel) {
  const int gray = scaleOverlayGray(gfx::YA::gray(pixel));
  const int alpha = scaleOverlayAlpha(gfx::YA::alpha(pixel));
  return qRgba(0, 0, gray, alpha);
}

void rgbaToOverlay(gfx::Surface<PixelRgba> overlay, gfx::CSurface<PixelRgba> source) {
  gfx::pureEach(overlay, source, rgbaToOverlayPx);
}

void paletteToOverlay(gfx::Surface<PixelRgba> overlay, gfx::CSurface<PixelIndex> source, PaletteCSpan palette) {
  gfx::pureEach(overlay, source, [palette](const PixelIndex pixel) {
    return paletteToOverlayPx(palette, pixel);
  });
}

void grayToOverlay(gfx::Surface<PixelRgba> overlay, gfx::CSurface<PixelGray> source) {
  gfx::pureEach(overlay, source, grayToOverlayPx);
}

}

void writeOverlay(
  PaletteCSpan palette,
  const Format format,
  QImage &overlay
) {
  gfx::Surface overlaySurface = makeSurface<PixelRgba>(overlay);
  switch (format) {
    case Format::rgba:
      return gfx::overFill(overlaySurface, rgbaToOverlayPx(0));
    case Format::index:
      return gfx::overFill(overlaySurface, paletteToOverlayPx(palette, 0));
    case Format::gray:
      return gfx::overFill(overlaySurface, grayToOverlayPx(0));
    default: Q_UNREACHABLE();
  }
}

void writeOverlay(
  PaletteCSpan palette,
  const Format format,
  QImage &overlay,
  const QImage &source
) {
  assert(overlay.size() == source.size());
  gfx::Surface overlaySurface = makeSurface<PixelRgba>(overlay);
  switch (format) {
    case Format::rgba:
      rgbaToOverlay(overlaySurface, makeSurface<PixelRgba>(source));
      break;
    case Format::index:
      paletteToOverlay(overlaySurface, makeSurface<PixelIndex>(source), palette);
      break;
    case Format::gray:
      grayToOverlay(overlaySurface, makeSurface<PixelGray>(source));
      break;
    default: Q_UNREACHABLE();
  }
}

void writeOverlay(
  PaletteCSpan palette,
  const Format format,
  QImage &overlay,
  const QImage &source,
  const QImage &mask
) {
  assert(overlay.size() == source.size());
  assert(overlay.size() == mask.size());
  writeOverlay(palette, format, overlay, source);
  gfx::maskClip(makeSurface<PixelRgba>(overlay), makeCSurface<PixelMask>(mask));
}

void growCel(Cel &cel, const Format format, const QRect rect) {
  SCOPE_TIME("growCel");
  
  if (!cel) {
    cel.img = {rect.size(), qimageFormat(format)};
    cel.pos = rect.topLeft();
    clearImage(cel.img);
    return;
  }
  const QRect celRect = cel.rect();
  if (!celRect.contains(rect)) {
    const QRect newRect = celRect.united(rect);
    QImage newImage{newRect.size(), cel.img.format()};
    clearImage(newImage);
    blitImage(newImage, cel.img, celRect.topLeft() - newRect.topLeft());
    cel.img = std::move(newImage);
    cel.pos = newRect.topLeft();
  }
}

void shrinkCel(Cel &cel, const QRect rect) {
  SCOPE_TIME("shrinkCel");
  
  if (!cel) return;
  if (rect.isEmpty()) return;
  if (!cel.rect().intersects(rect)) return;
  if (cel.rect().contains(rect, true)) return;
  
  QPoint min = toPoint(std::numeric_limits<int>::max());
  QPoint max = toPoint(std::numeric_limits<int>::min());
  visitSurface(cel.img, [&min, &max](auto image) {
    int y = 0;
    for (auto row : gfx::range(image)) {
      int x = 0;
      for (auto pixel : row) {
        if (pixel) {
          min.setX(std::min(min.x(), x));
          min.setY(std::min(min.y(), y));
          max.setX(std::max(max.x(), x));
          max.setY(std::max(max.y(), y));
        }
        ++x;
      }
      ++y;
    }
  });
  const QRect newRect{min, max};
  if (newRect.isEmpty()) {
    cel = {};
    return;
  }
  QImage image{newRect.size(), cel.img.format()};
  blitImage(image, cel.img, -newRect.topLeft());
  cel.img = std::move(image);
  cel.pos += newRect.topLeft();
}

QRgb sampleCel(const Cel &cel, QPoint pos) {
  if (cel.rect().contains(pos)) {
    pos -= cel.pos;
    const int depth = cel.img.depth() / 8;
    const int idx = pos.y() * cel.img.bytesPerLine() + pos.x() * depth;
    const uchar *bits = cel.img.bits() + idx;
    QRgb pixel = 0;
    std::memcpy(&pixel, bits, depth);
    return pixel;
  }
  return 0;
}
