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
#include <QtGui/qpainter.h>
#include "surface factory.hpp"

QImage compositeFrame(const Palette &palette, const Frame &frame, const LayerVisible &visible) {
  assert(!frame.empty());
  assert(frame.size() == visible.size());
  std::vector<Image> images;
  images.reserve(frame.size());
  for (size_t i = 0; i != frame.size(); ++i) {
    if (frame[i]) {
      images.push_back(frame[i]->outputImage());
    } else {
      images.push_back({});
    }
  }
  if (images.front().data.format() == QImage::Format_Grayscale8) {
    for (Image &image : images) {
      // @TODO Maybe don't use QImage
      image.data.reinterpretAsFormat(QImage::Format_Indexed8);
      image.data.setColorTable(QVector<QRgb>::fromStdVector(palette));
    }
  }
  QImage output{images.front().data.size(), QImage::Format_ARGB32};
  clearImage(output);
  
  // @TODO avoid using QPainter
  QPainter painter{&output};
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  for (size_t i = 0; i != frame.size(); ++i) {
    if (visible[i] && !images[i].data.isNull()) {
      painter.setTransform(getTransform(images[i]));
      painter.drawImage(0, 0, images[i].data);
    }
  }
  
  return output;
}

void compositeOverlay(QImage &drawing, const QImage &overlay) {
  porterDuff<ARGB_Format>(
    mode_src_over,
    makeSurface<QRgb>(drawing),
    makeSurface<QRgb>(overlay)
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

void blitTransformedImage(QImage &dst, const Image &src) {
  // @TODO avoid using QPainter
  QPainter painter{&dst};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setTransform(getTransform(src));
  painter.drawImage(0, 0, src.data);
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
