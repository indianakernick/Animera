//
//  composite.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "composite.hpp"

#include "config.hpp"
#include <QtGui/qpainter.h>
#include "pixel manip factory.hpp"

/*
#include <cmath>

struct Color {
  uint8_t r, g, b, a;
};

struct ColorF {
  float r, g, b, a;
};

float toFloat(const uint8_t component) {
  return static_cast<float>(component) / 255.0f;
}

uint8_t fromFloat(const float component) {
  return static_cast<uint8_t>(std::clamp(std::round(component * 255.0f), 0.0f, 255.0f));
}

ColorF toFloat(const Color color) {
  return {toFloat(color.r), toFloat(color.g), toFloat(color.b), toFloat(color.a)};
}

Color fromFloat(const ColorF color) {
  return {fromFloat(color.r), fromFloat(color.g), fromFloat(color.b), fromFloat(color.a)};
}

// straight alpha with float precision
Color compositeF(const Color aInt, const Color bInt, const uint8_t afInt, const uint8_t bfInt) {
  const float aF = toFloat(afInt);
  const float bF = toFloat(bfInt);
  const ColorF a = toFloat(aInt);
  const ColorF b = toFloat(bInt);
  
  const float cA = a.a*aF + b.a*bF;
  if (cA == 0.0f) {
    return {0, 0, 0, 0};
  } else {
    const float cR = (a.a*aF*a.r + b.a*bF*b.r) / cA;
    const float cG = (a.a*aF*a.g + b.a*bF*b.g) / cA;
    const float cB = (a.a*aF*a.b + b.a*bF*b.b) / cA;
    return fromFloat({cR, cG, cB, cA});
  }
}

// straight alpha with uint8 precision
Color compositeI(const Color a, const Color b, const uint8_t aF, const uint8_t bF) {
  const uint32_t cA = a.a*aF + b.a*bF;
  if (cA == 0) {
    return {0, 0, 0, 0};
  } else {
    const uint8_t cR = (a.a*aF*a.r + b.a*bF*b.r) / cA;
    const uint8_t cG = (a.a*aF*a.g + b.a*bF*b.g) / cA;
    const uint8_t cB = (a.a*aF*a.b + b.a*bF*b.b) / cA;
    return {cR, cG, cB, static_cast<uint8_t>(cA / 255)};
  }
}*/

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
      image.data.reinterpretAsFormat(QImage::Format_Indexed8);
      image.data.setColorTable(palette);
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
  // @TODO avoid using QPainter
  assert(drawing.size() == overlay.size());
  QPainter painter{&drawing};
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.drawImage(0, 0, overlay);
}

void blitImage(QImage &dst, const QImage &src, const QPoint pos) {
  // @TODO avoid using QPainter
  QPainter painter{&dst};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.drawImage(pos, src);
}

QImage blitImage(const QImage &src, const QRect rect) {
  // @TODO avoid using QPainter
  QImage dst{rect.size(), src.format()};
  QPainter painter{&dst};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.drawImage({0, 0}, src, rect);
  return dst;
}

void blitTransformedImage(QImage &dst, const Image &src) {
  QPainter painter{&dst};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setTransform(getTransform(src));
  painter.drawImage(0, 0, src.data);
}

void blitMaskImage(QImage &dst, const QImage &mask, const QImage &src, const QPoint pos) {
  assert(mask.size() == src.size());
  assert(mask.format() == mask_format);
  const QRect rect = mask.rect().intersected(dst.rect().translated(-pos));
  // @TODO correct but slow
  for (int y = rect.top(); y <= rect.bottom(); ++y) {
    for (int x = rect.left(); x <= rect.right(); ++x) {
      if (mask.pixel(x, y) == mask_color_on) {
        dst.setPixel(x + pos.x(), y + pos.y(), src.pixel(x, y));
      }
    }
  }
}

QImage blitMaskImage(const QImage &src, const QImage &mask, const QPoint pos) {
  assert(mask.format() == mask_format);
  const QRect rect = mask.rect().intersected(src.rect().translated(-pos));
  QImage dst{mask.size(), src.format()};
  // @TODO correct but slow
  for (int y = rect.top(); y <= rect.bottom(); ++y) {
    for (int x = rect.left(); x <= rect.right(); ++x) {
      if (mask.pixel(x, y) == mask_color_on) {
        dst.setPixel(x, y, src.pixel(x + pos.x(), y + pos.y()));
      }
    }
  }
  return dst;
}

namespace {

void colorToOverlay(QRgb &pixel) {
  const QRgb pxval = pixel;
  const int gray = qGray(qRed(pxval), qGreen(pxval), qBlue(pxval));
  const int alpha = std::max(tool_overlay_alpha_min, qAlpha(pxval) * 3 / 4);
  pixel = qRgba(gray, gray, gray, alpha);
}

uint32_t spread(const uint8_t byte) {
  int32_t i32 = byte;
  i32 <<= 24;
  i32 >>= 24; // C++20 says this is a sign extension
  return i32;
}

void applyMaskPixel(QRgb &pixel, const uchar mask) {
  pixel &= spread(mask);
}

}

void colorToOverlay(QImage &img) {
  assert(img.format() == getImageFormat(Format::color));
  img.detach();
  for (auto row : makePixelManip<uint32_t>(img).range()) {
    for (uint32_t &pixel : row) {
      colorToOverlay(pixel);
    }
  }
}

void colorToOverlay(QImage &img, const QImage &mask) {
  assert(img.format() == getImageFormat(Format::color));
  assert(img.size() == mask.size());
  img.detach();
  
  Range imgRange = makePixelManip<uint32_t>(img).range();
  auto maskRowIter = makePixelManip<uint8_t>(mask).range().begin();
  
  for (auto imgRow : imgRange) {
    const uint8_t *maskPixelIter = (*maskRowIter).begin();
    for (uint32_t &imgPixel : imgRow) {
      colorToOverlay(imgPixel);
      applyMaskPixel(imgPixel, *maskPixelIter);
      ++maskPixelIter;
    }
    ++maskRowIter;
  }
}
