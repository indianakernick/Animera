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
  auto sourceRowIter = source.range().begin();
  for (auto row : overlay.range()) {
    const PixelRgba *sourcePixel = (*sourceRowIter).begin();
    for (PixelRgba &pixel : row) {
      const int gray = qGray(*sourcePixel);
      const int alpha = scaleOverlayAlpha(qAlpha(*sourcePixel));
      pixel = qRgba(gray, gray, gray, alpha);
      ++sourcePixel;
    }
    ++sourceRowIter;
  }
}

void paletteToOverlay(Surface<PixelRgba> overlay, CSurface<PixelIndex> source, PaletteCSpan palette) {
  auto sourceRowIter = source.range().begin();
  for (auto row : overlay.range()) {
    const PixelIndex *sourcePixel = (*sourceRowIter).begin();
    for (PixelRgba &pixel : row) {
      const QRgb color = palette[*sourcePixel];
      const int gray = qGray(color);
      pixel = qRgba(gray, gray, gray, scaleOverlayAlpha(qAlpha(color)));
      ++sourcePixel;
    }
    ++sourceRowIter;
  }
}

void grayToOverlay(Surface<PixelRgba> overlay, CSurface<PixelGray> source) {
  auto sourceRowIter = source.range().begin();
  for (auto row : overlay.range()) {
    const PixelGray *sourcePixel = (*sourceRowIter).begin();
    for (PixelRgba &pixel : row) {
      const int gray = *sourcePixel;
      pixel = qRgba(0, 0, scaleOverlayGray(gray), scaleOverlayAlpha(255));
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

QImage grayToMono(const QImage &src) {
  // @TODO libpng
  // This is sooooooo inefficient
  assert(src.format() == qimageFormat(Format::gray));
  QImage dst{src.size(), QImage::Format_Mono};
  for (int y = 0; y != dst.height(); ++y) {
    for (int x = 0; x != dst.width(); ++x) {
      dst.setPixel(x, y, static_cast<uint8_t>(src.pixel(x, y)) < 128 ? 0 : 1);
    }
  }
  return dst;
}
