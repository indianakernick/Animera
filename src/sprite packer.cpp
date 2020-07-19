//
//  sprite packer.cpp
//  Animera
//
//  Created by Indiana Kernick on 19/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "sprite packer.hpp"

#include "composite.hpp"
#include "export png.hpp"
#include <QtCore/qmath.h>

namespace {

Format toCanvasFormat(const PixelFormat format) {
  return format == PixelFormat::rgba ? Format::rgba : Format::gray;
}

}

void SpritePacker::init(const PixelFormat newFormat) {
  texture = {};
  rects.clear();
  area = 0;
  format = newFormat;
}

void SpritePacker::append(std::size_t count, const QSize size) {
  stbrp_rect rect;
  rect.w = size.width() + 2 * padding;
  rect.h = size.height() + 2 * padding;
  area += count * rect.w * rect.h;
  while (count--) {
    rects.push_back(rect);
  }
}

void SpritePacker::appendWhite() {
  append(1, {1, 1});
}

Error SpritePacker::pack() {
  int length = qNextPowerOfTwo(qRound(std::sqrt(area)));
  std::vector<stbrp_node> nodes;
  stbrp_context ctx;
  int packed = 0;
  const int rectCount = static_cast<int>(rects.size());
  
  while (length <= 65536) {
    nodes.resize(length);
    stbrp_init_target(&ctx, length, length, nodes.data(), length);
    packed = stbrp_pack_rects(&ctx, rects.data(), rectCount);
    if (packed) break;
    length *= 2;
  }
  
  if (!packed) {
    return "Failed to pack rectangles";
  }
  texture = QImage{length, length, qimageFormat(toCanvasFormat(format))};
  return {};
}

QRect SpritePacker::copy(const std::size_t i, const QImage &image) {
  assert(!image.isNull());
  assert(!texture.isNull());
  assert(image.format() == texture.format());
  const QRect r = rect(i);
  assert(r.size() == image.size());
  blitImage(texture, image, r.topLeft());
  return r;
}

QRect SpritePacker::copyWhite(const std::size_t i) {
  assert(!texture.isNull());
  const QRect r = rect(i);
  assert((r.size() == QSize{1, 1}));
  texture.setPixel(r.topLeft(), 0xFFFFFFFF);
  return r;
}

Error SpritePacker::writePng(QIODevice &dev) {
  return exportCelPng(dev, {}, texture, toCanvasFormat(format), format);
}
