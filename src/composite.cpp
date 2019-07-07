//
//  composite.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "composite.hpp"

#include "config.hpp"
#include "masking.hpp"
#include "formats.hpp"
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

void compositeColor(Surface<PixelColor> output, const Frame &frame) {
  eachImage(frame, [output](const QImage &image) {
    porterDuff(
      mode_src_over,
      output,
      makeCSurface<PixelColor>(image),
      FormatARGB{},
      FormatARGB{}
    );
  });
}

void compositePalette(Surface<PixelColor> output, const Frame &frame, PaletteCSpan palette) {
  FormatPalette format{palette.data()};
  eachImage(frame, [output, format](const QImage &image) {
    porterDuff(
      mode_src_over,
      output,
      makeCSurface<PixelPalette>(image),
      FormatARGB{},
      format
    );
  });
}

void compositeGray(Surface<PixelColor> output, const Frame &frame) {
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
  QImage output{size, qimageFormat(Format::color)};
  clearImage(output);
  Surface<PixelColor> outputSurface = makeSurface<PixelColor>(output);
  
  switch (format) {
    case Format::color:
      compositeColor(outputSurface, frame);
      break;
    case Format::palette:
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
    makeSurface<PixelColor>(drawing),
    makeSurface<PixelColor>(overlay),
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

constexpr int convertAlpha(const int alpha) {
  return std::max(tool_overlay_alpha_min, alpha * 3 / 4);
}

void colorToOverlay(Surface<PixelColor> overlay, CSurface<PixelColor> source) {
  auto sourceRowIter = source.range().begin();
  for (auto row : overlay.range()) {
    const PixelColor *sourcePixel = (*sourceRowIter).begin();
    for (PixelColor &pixel : row) {
      const int gray = qGray(*sourcePixel);
      const int alpha = convertAlpha(qAlpha(*sourcePixel));
      pixel = qRgba(gray, gray, gray, alpha);
      ++sourcePixel;
    }
    ++sourceRowIter;
  }
}

void paletteToOverlay(Surface<PixelColor> overlay, CSurface<PixelPalette> source, PaletteCSpan palette) {
  auto sourceRowIter = source.range().begin();
  for (auto row : overlay.range()) {
    const PixelPalette *sourcePixel = (*sourceRowIter).begin();
    for (PixelColor &pixel : row) {
      const QRgb color = palette[*sourcePixel];
      const int gray = qGray(color);
      pixel = qRgba(gray, gray, gray, convertAlpha(qAlpha(color)));
      ++sourcePixel;
    }
    ++sourceRowIter;
  }
}

void grayToOverlay(Surface<PixelColor> overlay, CSurface<PixelGray> source) {
  auto sourceRowIter = source.range().begin();
  for (auto row : overlay.range()) {
    const PixelGray *sourcePixel = (*sourceRowIter).begin();
    for (PixelColor &pixel : row) {
      const int gray = *sourcePixel;
      pixel = qRgba(gray, gray, gray, convertAlpha(255));
      ++sourcePixel;
    }
    ++sourceRowIter;
  }
}

}

void writeOverlay(
  PaletteCSpan palette,
  const Format format,
  QImage &overlay,
  const QImage &source
) {
  assert(overlay.size() == source.size());
  Surface overlaySurface = makeSurface<PixelColor>(overlay);
  switch (format) {
    case Format::color:
      colorToOverlay(overlaySurface, makeSurface<PixelColor>(source));
      break;
    case Format::palette:
      paletteToOverlay(overlaySurface, makeSurface<PixelPalette>(source), palette);
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
  maskClip(makeSurface<PixelColor>(overlay), makeCSurface<PixelMask>(mask));
}
