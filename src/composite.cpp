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

std::vector<Image> getImages(const Frame &frame, const LayerVisible &visible) {
  assert(frame.size() == visible.size());
  std::vector<Image> images;
  images.reserve(frame.size());
  for (size_t i = 0; i != frame.size(); ++i) {
    if (frame[i] && !frame[i]->image.data.isNull() && visible[i]) {
      images.push_back(frame[i]->image);
    }
  }
  return images;
}

void compositeColor(Surface<QRgb> output, const std::vector<Image> &images) {
  // Layer 0 is on top of layer 1
  for (auto i = images.rbegin(); i != images.rend(); ++i) {
    porterDuff(
      mode_src_over,
      output,
      makeCSurface<QRgb>(i->data),
      FormatARGB{},
      FormatARGB{}
    );
  }
}

void compositePalette(Surface<QRgb> output, const std::vector<Image> &images, const Palette *palette) {
  assert(palette);
  FormatPalette format{palette->data(), palette->size()};
  for (auto i = images.rbegin(); i != images.rend(); ++i) {
    porterDuff(
      mode_src_over,
      output,
      makeCSurface<uint8_t>(i->data),
      FormatARGB{},
      format
    );
  }
}

void compositeGray(Surface<QRgb> output, const std::vector<Image> &images) {
  for (auto i = images.rbegin(); i != images.rend(); ++i) {
    porterDuff(
      mode_src_over,
      output,
      makeCSurface<uint8_t>(i->data),
      FormatARGB{},
      FormatGray{}
    );
  }
}

}

QImage compositeFrame(
  const Palette *palette,
  const Frame &frame,
  const LayerVisible &visible,
  const QSize size,
  const Format format
) {
  std::vector<Image> images = getImages(frame, visible);
  QImage output{size, QImage::Format_ARGB32};
  clearImage(output);
  Surface<QRgb> outputSurface = makeSurface<QRgb>(output);
  
  switch (format) {
    case Format::color:
      compositeColor(outputSurface, images);
      break;
    case Format::palette:
      compositePalette(outputSurface, images, palette);
      break;
    case Format::gray:
      compositeGray(outputSurface, images);
      break;
  }

  return output;
}

void compositeOverlay(QImage &drawing, const QImage &overlay) {
  porterDuff(
    mode_src_over,
    makeSurface<QRgb>(drawing),
    makeSurface<QRgb>(overlay),
    FormatARGB{},
    FormatARGB{}
  );
}

void blitImage(QImage &dst, const QImage &src, const QPoint pos) {
  copyRegion(
    makeSurface<QRgb>(dst),
    makeSurface<QRgb>(src),
    pos
  );
}

QImage blitImage(const QImage &src, const QRect rect) {
  // @TODO does it really make sense allocate a new QImage?
  QImage dst{rect.size(), src.format()};
  copyRegion(
    makeSurface<QRgb>(dst),
    makeSurface<QRgb>(src),
    -rect.topLeft()
  );
  return dst;
}

void blitMaskImage(QImage &dst, const QImage &mask, const QImage &src, const QPoint pos) {
  assert(mask.size() == src.size());
  maskCopyRegion(
    makeSurface<QRgb>(dst),
    makeSurface<QRgb>(src),
    makeSurface<uint8_t>(mask),
    pos,
    pos
  );
}

QImage blitMaskImage(const QImage &src, const QImage &mask, const QPoint pos) {
  // @TODO does it really make sense allocate a new QImage?
  QImage dst{mask.size(), src.format()};
  maskCopyRegion(
    makeSurface<QRgb>(dst),
    makeSurface<QRgb>(src),
    makeSurface<uint8_t>(mask),
    -pos,
    {0, 0}
  );
  return dst;
}

namespace {

void colorToOverlay(const Surface<QRgb> surface) {
  for (auto row : surface.range()) {
    for (QRgb &pixel : row) {
      const int gray = qGray(pixel);
      const int alpha = std::max(tool_overlay_alpha_min, qAlpha(pixel) * 3 / 4);
      pixel = qRgba(gray, gray, gray, alpha);
    }
  }
}

}

void colorToOverlay(QImage &img) {
  assert(img.format() == getImageFormat(Format::color));
  colorToOverlay(makeSurface<QRgb>(img));
}

void colorToOverlay(QImage &img, const QImage &mask) {
  assert(img.format() == getImageFormat(Format::color));
  assert(img.size() == mask.size());
  const Surface surface = makeSurface<QRgb>(img);
  colorToOverlay(surface);
  maskClip(surface, makeCSurface<uint8_t>(mask));
}
