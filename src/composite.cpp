//
//  composite.cpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "composite.hpp"

#include <limits>
#include "config.hpp"
#include <Graphics/fill.hpp>
#include <Graphics/mask.hpp>
#include "surface factory.hpp"
#include "graphics convert.hpp"
#include <Graphics/transform.hpp>
#include <Graphics/porter duff.hpp>

namespace {

template <typename Func>
void eachImage(const Frame &frame, Func func) {
  // Layer 0 is on top of layer 1
  for (auto c = frame.crbegin(); c != frame.crend(); ++c) {
    const Cell &cell = **c;
    if (cell) {
      // @TODO remove
      QImage dup = cell.image;
      dup.fill(qRgba(127, 127, 127, 63));
      func(dup);
      func(cell.image);
    }
  }
}

template <typename Format>
using Surface = gfx::Surface<gfx::Pixel<Format>>;

template <typename Format>
void compositeColor(Surface<Format> output, const Frame &frame) {
  eachImage(frame, [output](const QImage &image) {
    gfx::porterDuffRegion(
      gfx::mode_src_over,
      output,
      makeCSurface<PixelRgba>(image),
      Format{},
      gfx::ARGB{},
      convert(image.offset())
    );
  });
}

template <typename Format>
void compositePalette(Surface<Format> output, const Frame &frame, PaletteCSpan palette) {
  gfx::I<> format{palette.data()};
  eachImage(frame, [output, format](const QImage &image) {
    gfx::porterDuffRegion(
      gfx::mode_src_over,
      output,
      makeCSurface<PixelIndex>(image),
      Format{},
      format,
      convert(image.offset())
    );
  });
}

template <typename Format>
void compositeGray(Surface<Format> output, const Frame &frame) {
  eachImage(frame, [output](const QImage &image) {
    gfx::porterDuffRegion(
      gfx::mode_src_over,
      output,
      makeCSurface<PixelGray>(image),
      Format{},
      gfx::YA{},
      convert(image.offset())
    );
  });
}

}

template <typename PxFmt>
void compositeFrame(
  QImage &dst,
  PaletteCSpan palette,
  const Frame &frame,
  const Format format
) {
  auto dstSurface = makeSurface<typename PxFmt::Pixel>(dst);
  
  switch (format) {
    case Format::rgba:
      compositeColor<PxFmt>(dstSurface, frame);
      break;
    case Format::index:
      compositePalette<PxFmt>(dstSurface, frame, palette);
      break;
    case Format::gray:
      compositeGray<PxFmt>(dstSurface, frame);
      break;
    default: Q_UNREACHABLE();
  }
}

template void compositeFrame<gfx::ARGB>(QImage &, PaletteCSpan, const Frame &, Format);
template void compositeFrame<gfx::YA>(QImage &, PaletteCSpan, const Frame &, Format);

void compositeOverlay(QImage &drawing, const QImage &overlay) {
  porterDuff(
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

QImage blitImage(const QImage &src, const QRect rect) {
  // @TODO does it really make sense allocate a new QImage?
  QImage dst{rect.size(), src.format()};
  visitSurfaces(dst, src, [pos = rect.topLeft()](auto dst, auto src) {
    gfx::overFill(dst);
    gfx::copyRegion(dst, src, convert(-pos));
  });
  return dst;
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

QImage blitMaskImage(const QImage &src, const QImage &mask, const QPoint pos) {
  // @TODO does it really make sense allocate a new QImage?
  QImage dst{mask.size(), src.format()};
  visitSurfaces(dst, src, [&mask, pos](auto dst, auto src) {
    gfx::overFill(dst);
    gfx::maskCopyRegion(
      dst,
      src,
      makeSurface<PixelMask>(mask),
      convert(-pos),
      {0, 0}
    );
  });
  return dst;
}

void fillMaskImage(QImage &dst, const QImage &mask, const QRgb color, const QPoint pos) {
  visitSurface(dst, [&mask, pos, color](auto dst) {
    gfx::maskFillRegion(dst, makeCSurface<PixelMask>(mask), color, convert(pos));
  });
}

namespace {

PixelRgba rgbaToOverlayPx(const PixelRgba pixel) {
  const int gray = qGray(pixel);
  const int alpha = scaleOverlayAlpha(qAlpha(pixel));
  return qRgba(gray, gray, gray, alpha);
}

PixelRgba paletteToOverlayPx(const PaletteCSpan palette, const PixelIndex pixel) {
  return rgbaToOverlayPx(palette[pixel]);
}

PixelRgba grayToOverlayPx(const PixelGray pixel) {
  const int gray = gfx::YA::gray(pixel);
  const int alpha = gfx::YA::alpha(pixel);
  return qRgba(0, 0, scaleOverlayGray(gray), scaleOverlayAlpha(alpha));
}

void rgbaToOverlay(gfx::Surface<PixelRgba> overlay, gfx::CSurface<PixelRgba> source) {
  pixelTransform(overlay, source, rgbaToOverlayPx);
}

void paletteToOverlay(gfx::Surface<PixelRgba> overlay, gfx::CSurface<PixelIndex> source, PaletteCSpan palette) {
  pixelTransform(overlay, source, [palette](const PixelIndex pixel) {
    return paletteToOverlayPx(palette, pixel);
  });
}

void grayToOverlay(gfx::Surface<PixelRgba> overlay, gfx::CSurface<PixelGray> source) {
  pixelTransform(overlay, source, grayToOverlayPx);
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

void growCell(Cell &cell, const Format format, const QRect rect) {
  if (!cell) {
    cell.image = {rect.size(), qimageFormat(format)};
    cell.image.setOffset(rect.topLeft());
    clearImage(cell.image);
    return;
  }
  const QRect cellRect = {cell.image.offset(), cell.image.size()};
  if (!cellRect.contains(rect)) {
    const QRect newRect = cellRect.united(rect);
    QImage newImage{newRect.size(), cell.image.format()};
    clearImage(newImage);
    blitImage(newImage, cell.image, cellRect.topLeft() - newRect.topLeft());
    newImage.setOffset(newRect.topLeft());
    cell.image = std::move(newImage);
  }
}

// @TODO call this before saving
// this might create some null cells so the timeline could be optimized too
// while we're at it, we could optimize palette as well
void optimizeCell(Cell &cell) {
  if (!cell) return;
  QPoint min = toPoint(std::numeric_limits<int>::max());
  QPoint max = toPoint(std::numeric_limits<int>::min());
  visitSurface(cell.image, [&min, &max](auto image) {
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
    cell.image = {};
    return;
  }
  QImage newImage{newRect.size(), cell.image.format()};
  blitImage(newImage, cell.image, newRect.topLeft());
  newImage.setOffset(newRect.topLeft() + cell.image.offset());
  cell.image = std::move(newImage);
}

QRgb sampleCell(const Cell &cell, const QPoint pos) {
  return cell.rect().contains(pos) ? cell.image.pixel(pos - cell.pos()) : 0;
}
