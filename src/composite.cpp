//
//  composite.cpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "composite.hpp"

#include "config.hpp"
#include "masking.hpp"
#include "formats.hpp"
#include "transform.hpp"
#include "porter duff.hpp"
#include "surface factory.hpp"

namespace {

template <typename Func>
void eachImage(const Frame &frame, Func func) {
  // Layer 0 is on top of layer 1
  for (auto c = frame.crbegin(); c != frame.crend(); ++c) {
    func((*c)->image);
  }
}

void compositeColor(Surface<PixelRgba> output, const Frame &frame) {
  eachImage(frame, [output](const QImage &image) {
    porterDuff(
      mode_src_over,
      output,
      makeCSurface<PixelRgba>(image),
      FormatARGB{},
      FormatARGB{}
    );
  });
}

void compositePalette(Surface<PixelRgba> output, const Frame &frame, PaletteCSpan palette) {
  FormatPalette format{palette.data()};
  eachImage(frame, [output, format](const QImage &image) {
    porterDuff(
      mode_src_over,
      output,
      makeCSurface<PixelIndex>(image),
      FormatARGB{},
      format
    );
  });
}

void compositeGray(Surface<PixelRgba> output, const Frame &frame) {
  eachImage(frame, [output](const QImage &image) {
    porterDuff(
      mode_src_over,
      output,
      makeCSurface<PixelGray>(image),
      FormatARGB{},
      FormatGray{}
    );
  });
}

}

QImage compositeFrame(
  PaletteCSpan palette,
  const Frame &frame,
  const QSize size,
  const Format format
) {
  QImage output{size, qimageFormat(Format::rgba)};
  clearImage(output);
  auto outputSurface = makeSurface<PixelRgba>(output);
  
  switch (format) {
    case Format::rgba:
      compositeColor(outputSurface, frame);
      break;
    case Format::index:
      compositePalette(outputSurface, frame, palette);
      break;
    case Format::gray:
      compositeGray(outputSurface, frame);
      break;
    default: Q_UNREACHABLE();
  }

  return output;
}

void compositeOverlay(QImage &drawing, const QImage &overlay) {
  porterDuff(
    mode_src_over,
    makeSurface<PixelRgba>(drawing),
    makeSurface<PixelRgba>(overlay),
    FormatARGB{},
    FormatARGB{}
  );
}

void blitImage(QImage &dst, const QImage &src, const QPoint pos) {
  visitSurfaces(dst, src, [pos](auto dstSurface, auto srcSurface) {
    copyRegion(
      dstSurface,
      srcSurface,
      pos
    );
  });
}

QImage blitImage(const QImage &src, const QRect rect) {
  // @TODO does it really make sense allocate a new QImage?
  QImage dst{rect.size(), src.format()};
  visitSurfaces(dst, src, [pos = rect.topLeft()](auto dstSurface, auto srcSurface) {
    dstSurface.overFill();
    copyRegion(
      dstSurface,
      srcSurface,
      -pos
    );
  });
  return dst;
}

void blitMaskImage(QImage &dst, const QImage &mask, const QImage &src, const QPoint pos) {
  assert(mask.size() == src.size());
  visitSurfaces(dst, src, [&mask, pos](auto dstSurface, auto srcSurface) {
    maskCopyRegion(
      dstSurface,
      srcSurface,
      makeSurface<PixelMask>(mask),
      pos,
      pos
    );
  });
}

QImage blitMaskImage(const QImage &src, const QImage &mask, const QPoint pos) {
  // @TODO does it really make sense allocate a new QImage?
  QImage dst{mask.size(), src.format()};
  visitSurfaces(dst, src, [&mask, pos](auto dstSurface, auto srcSurface) {
    dstSurface.overFill();
    maskCopyRegion(
      dstSurface,
      srcSurface,
      makeSurface<PixelMask>(mask),
      -pos,
      {0, 0}
    );
  });
  return dst;
}

namespace {

void rgbaToOverlay(Surface<PixelRgba> overlay, CSurface<PixelRgba> source) {
  pixelTransform(overlay, source, [](const PixelRgba pixel) {
    const int gray = qGray(pixel);
    const int alpha = scaleOverlayAlpha(qAlpha(pixel));
    return qRgba(gray, gray, gray, alpha);
  });
}

void paletteToOverlay(Surface<PixelRgba> overlay, CSurface<PixelIndex> source, PaletteCSpan palette) {
  pixelTransform(overlay, source, [palette](const PixelIndex pixel) {
    const QRgb color = palette[pixel];
    const int gray = qGray(color);
    return qRgba(gray, gray, gray, scaleOverlayAlpha(qAlpha(color)));
  });
}

void grayToOverlay(Surface<PixelRgba> overlay, CSurface<PixelGray> source) {
  pixelTransform(overlay, source, [](const PixelGray pixel) {
    const int gray = FormatGray::toGray(pixel);
    const int alpha = FormatGray::toAlpha(pixel);
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
  Surface overlaySurface = makeSurface<PixelRgba>(overlay);
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
  maskClip(makeSurface<PixelRgba>(overlay), makeCSurface<PixelMask>(mask));
}
