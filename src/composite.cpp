//
//  composite.cpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "composite.hpp"

#include "config.hpp"
#include <Graphics/fill.hpp>
#include <Graphics/mask.hpp>
#include "surface factory.hpp"
#include <Graphics/transform.hpp>
#include <Graphics/porter duff.hpp>

namespace {

template <typename Func>
void eachImage(const Frame &frame, Func func) {
  // Layer 0 is on top of layer 1
  for (auto c = frame.crbegin(); c != frame.crend(); ++c) {
    func((*c)->image);
  }
}

template <typename Format>
void compositeColor(gfx::Surface<gfx::Pixel<Format>> output, const Frame &frame) {
  eachImage(frame, [output](const QImage &image) {
    porterDuff(
      gfx::mode_src_over,
      output,
      makeCSurface<PixelRgba>(image),
      Format{},
      gfx::ARGB{}
    );
  });
}

template <typename Format>
void compositePalette(gfx::Surface<gfx::Pixel<Format>> output, const Frame &frame, PaletteCSpan palette) {
  gfx::I<> format{palette.data()};
  eachImage(frame, [output, format](const QImage &image) {
    porterDuff(
      gfx::mode_src_over,
      output,
      makeCSurface<PixelIndex>(image),
      Format{},
      format
    );
  });
}

template <typename Format>
void compositeGray(gfx::Surface<gfx::Pixel<Format>> output, const Frame &frame) {
  eachImage(frame, [output](const QImage &image) {
    porterDuff(
      gfx::mode_src_over,
      output,
      makeCSurface<PixelGray>(image),
      Format{},
      gfx::YA{}
    );
  });
}

template <typename Pixel>
QImage::Format qimageFormat();

template <>
QImage::Format qimageFormat<uint8_t>() {
  return QImage::Format_Grayscale8;
}

template <>
QImage::Format qimageFormat<uint16_t>() {
  return QImage::Format_Grayscale16;
}

template <>
QImage::Format qimageFormat<uint32_t>() {
  return QImage::Format_ARGB32;
}

}

template <typename PxFmt>
QImage compositeFrame(
  PaletteCSpan palette,
  const Frame &frame,
  const QSize size,
  const Format format
) {
  QImage output{size, qimageFormat<typename PxFmt::Pixel>()};
  clearImage(output);
  auto outputSurface = makeSurface<typename PxFmt::Pixel>(output);
  
  switch (format) {
    case Format::rgba:
      compositeColor<PxFmt>(outputSurface, frame);
      break;
    case Format::index:
      compositePalette<PxFmt>(outputSurface, frame, palette);
      break;
    case Format::gray:
      compositeGray<PxFmt>(outputSurface, frame);
      break;
    default: Q_UNREACHABLE();
  }

  return output;
}

template QImage compositeFrame<gfx::ARGB>(PaletteCSpan, const Frame &, QSize, Format);
template QImage compositeFrame<gfx::YA>(PaletteCSpan, const Frame &, QSize, Format);

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
  visitSurfaces(dst, src, [pos](auto dstSurface, auto srcSurface) {
    gfx::copyRegion(
      dstSurface,
      srcSurface,
      convert(pos)
    );
  });
}

QImage blitImage(const QImage &src, const QRect rect) {
  // @TODO does it really make sense allocate a new QImage?
  QImage dst{rect.size(), src.format()};
  visitSurfaces(dst, src, [pos = rect.topLeft()](auto dstSurface, auto srcSurface) {
    gfx::overFill(dstSurface);
    gfx::copyRegion(
      dstSurface,
      srcSurface,
      convert(-pos)
    );
  });
  return dst;
}

void blitMaskImage(QImage &dst, const QImage &mask, const QImage &src, const QPoint pos) {
  assert(mask.size() == src.size());
  visitSurfaces(dst, src, [&mask, pos](auto dstSurface, auto srcSurface) {
    gfx::maskCopyRegion(
      dstSurface,
      srcSurface,
      makeSurface<PixelMask>(mask),
      convert(pos),
      convert(pos)
    );
  });
}

QImage blitMaskImage(const QImage &src, const QImage &mask, const QPoint pos) {
  // @TODO does it really make sense allocate a new QImage?
  QImage dst{mask.size(), src.format()};
  visitSurfaces(dst, src, [&mask, pos](auto dstSurface, auto srcSurface) {
    gfx::overFill(dstSurface);
    gfx::maskCopyRegion(
      dstSurface,
      srcSurface,
      makeSurface<PixelMask>(mask),
      convert(-pos),
      {0, 0}
    );
  });
  return dst;
}

namespace {

void rgbaToOverlay(gfx::Surface<PixelRgba> overlay, gfx::CSurface<PixelRgba> source) {
  pixelTransform(overlay, source, [](const PixelRgba pixel) {
    const int gray = qGray(pixel);
    const int alpha = scaleOverlayAlpha(qAlpha(pixel));
    return qRgba(gray, gray, gray, alpha);
  });
}

void paletteToOverlay(gfx::Surface<PixelRgba> overlay, gfx::CSurface<PixelIndex> source, PaletteCSpan palette) {
  pixelTransform(overlay, source, [palette](const PixelIndex pixel) {
    const QRgb color = palette[pixel];
    const int gray = qGray(color);
    return qRgba(gray, gray, gray, scaleOverlayAlpha(qAlpha(color)));
  });
}

void grayToOverlay(gfx::Surface<PixelRgba> overlay, gfx::CSurface<PixelGray> source) {
  pixelTransform(overlay, source, [](const PixelGray pixel) {
    const int gray = gfx::YA::gray(pixel);
    const int alpha = gfx::YA::alpha(pixel);
    return qRgba(0, 0, scaleOverlayGray(gray), scaleOverlayAlpha(alpha));
  });
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
