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
#include <Graphics/copy.hpp>
#include <Graphics/each.hpp>
#include <Graphics/traits.hpp>
#include "surface factory.hpp"
#include "graphics convert.hpp"

SpritePacker::SpritePacker(const DataFormat dataFormat)
  : dataFormat{dataFormat} {}

void SpritePacker::init(const PixelFormat newFormat) {
  texture = {};
  rects.clear();
  area = 0;
  pixelFormat = newFormat;
}

void SpritePacker::append(const QSize size) {
  stbrp_rect rect;
  if (size.isEmpty()) {
    rect.w = 0;
    rect.h = 0;
  } else {
    rect.w = size.width() + 2 * padding;
    rect.h = size.height() + 2 * padding;
    area += rect.w * rect.h;
  }
  rects.push_back(rect);
}

void SpritePacker::appendWhite() {
  append({1, 1});
}

namespace {

QImage::Format toImageFormat(const PixelFormat format) {
  switch (format) {
    case PixelFormat::rgba:       return QImage::Format_ARGB32;
    case PixelFormat::index:      return QImage::Format_Grayscale8;
    case PixelFormat::gray:       return QImage::Format_Grayscale8;
    case PixelFormat::gray_alpha: return QImage::Format_Grayscale16;
    case PixelFormat::monochrome: return QImage::Format_Mono;
  }
}

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
  texture = QImage{length, length, toImageFormat(pixelFormat)};
  clearImage(texture);
  return {};
}

Error SpritePacker::setFormat(const Format newFormat, const PaletteCSpan newPalette) {
  palette = newPalette;
  copyFunc = getCopyFunc(newFormat);
  if (!copyFunc) {
    return "Chosen pixel format conversion is not supported";
  }
  return {};
}

QRect SpritePacker::copy(const std::size_t i, const QImage &image) {
  assert(!image.isNull());
  assert(!texture.isNull());
  const QRect r = rect(i);
  assert(r.size() == image.size());
  assert(copyFunc);
  (this->*copyFunc)(image, r.topLeft());
  return r;
}

QRect SpritePacker::copyWhite(const std::size_t i) {
  assert(!texture.isNull());
  const QRect r = rect(i);
  assert((r.size() == QSize{1, 1}));
  texture.setPixel(r.topLeft(), 0xFFFFFFFF);
  return r;
}

Error SpritePacker::write(QIODevice &dev) {
  switch (dataFormat) {
    case DataFormat::png:
      return exportPng(dev, palette, texture, pixelFormat);
    case DataFormat::raw:
      return visitSurface(texture, [&](auto src) {
        const qint64 width = src.byteWidth();
        for (auto row : src) {
          if (dev.write(reinterpret_cast<const char *>(row.begin()), width) != width) {
            return Error{"Error writing image data"};
          }
        }
        return Error{};
      });
  }
}

std::size_t SpritePacker::count() const {
  return rects.size();
}

int SpritePacker::width() const {
  return texture.width();
}

int SpritePacker::height() const {
  return texture.height();
}

QRect SpritePacker::rect(const std::size_t i) const {
  assert(i < rects.size());
  return {
    rects[i].x + padding, rects[i].y + padding,
    rects[i].w - 2 * padding, rects[i].h - 2 * padding
  };
}

SpritePacker::CopyFunc SpritePacker::getCopyFunc(const Format canvasFormat) const {
  switch (pixelFormat) {
    case PixelFormat::rgba:
      switch (canvasFormat) {
        case Format::rgba:
          return &SpritePacker::copyRgbaToRgba;
        case Format::index:
          return &SpritePacker::copyIndexToRgba;
        case Format::gray:
          return &SpritePacker::copyGrayToRgba;
      }
    case PixelFormat::index:
      return nullptr;
    case PixelFormat::gray:
      switch (canvasFormat) {
        case Format::rgba:
        case Format::index:
          return nullptr;
        case Format::gray:
          return &SpritePacker::copyGrayToGray;
      }
    case PixelFormat::gray_alpha:
      switch (canvasFormat) {
        case Format::rgba:
        case Format::index:
          return nullptr;
        case Format::gray:
          return &SpritePacker::copyGrayToGrayAlpha;
      }
      break;
    case PixelFormat::monochrome:
      return nullptr;
  }
}

namespace {

template <typename DstFmt, typename SrcFmt>
void copyConvert(QImage &dstImage, const QImage &srcImage, const QPoint pos, SrcFmt srcFmt) {
  gfx::Surface dst = makeSurface<gfx::Pixel<DstFmt>>(dstImage);
  gfx::Surface src = makeCSurface<gfx::Pixel<SrcFmt>>(srcImage);
  if constexpr (std::is_same_v<DstFmt, SrcFmt>) {
    gfx::copyRegion(dst, src, convert(pos));
  } else {
    gfx::eachRegion(dst, src, convert(pos), [srcFmt](auto &dst, const auto src) {
      dst = DstFmt::pixel(srcFmt.color(src));
    });
  }
}

}

void SpritePacker::copyRgbaToRgba(const QImage &image, const QPoint pos) {
  copyConvert<gfx::ABGR>(texture, image, pos, FmtRgba{});
}

void SpritePacker::copyIndexToRgba(const QImage &image, const QPoint pos) {
  copyConvert<gfx::ABGR>(texture, image, pos, FmtIndex{&palette[0].underlying()});
}

void SpritePacker::copyGrayToRgba(const QImage &image, const QPoint pos) {
  copyConvert<gfx::ABGR>(texture, image, pos, FmtGray{});
}

void SpritePacker::copyGrayToGray(const QImage &image, const QPoint pos) {
  copyConvert<gfx::Y>(texture, image, pos, FmtGray{});
}

void SpritePacker::copyGrayToGrayAlpha(const QImage &image, const QPoint pos) {
  copyConvert<FmtGray>(texture, image, pos, FmtGray{});
}
