//
//  composite.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "composite.hpp"

#include <QtGui/qpainter.h>

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

namespace {

void paintImage(QPainter &painter, const Image &img) {
  painter.setTransform(getTransform(img));
  painter.drawImage(0, 0, img.data);
}

}

QImage compositeFrame(const Palette &palette, const Frame &frame) {
  assert(!frame.empty());
  std::vector<Image> images;
  images.reserve(frame.size());
  for (const Cell *cell : frame) {
    images.push_back(cell->outputImage());
  }
  if (images.front().data.format() == QImage::Format_Grayscale8) {
    for (Image &image : images) {
      image.data.reinterpretAsFormat(QImage::Format_Indexed8);
      image.data.setColorTable(palette);
    }
  }
  QImage output{images.front().data.size(), QImage::Format_ARGB32};
  output.fill(0);
  
  QPainter painter{&output};
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  for (const Image &image : images) {
    paintImage(painter, image);
  }
  
  return output;
}

void compositeOverlay(QImage &drawing, const QImage &overlay) {
  assert(drawing.size() == overlay.size());
  QPainter painter{&drawing};
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.drawImage(0, 0, overlay);
}
