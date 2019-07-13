//
//  palette.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "palette.hpp"

#include <QtCore/qiodevice.h>

namespace {

constexpr int quantColor(const double size, const int color) {
  return 255 * color / size + 0.5;
}

constexpr QRgb quantColor(const int size, const int r, const int g, const int b) {
  return qRgb(quantColor(size, r), quantColor(size, g), quantColor(size, b));
}

constexpr QRgb quantGray(const int size, const int y) {
  const int gray = quantColor(size, y);
  return qRgb(gray, gray, gray);
}

constexpr std::array<QRgb, 24> hue_palette = {
  quantColor(4, 4, 0, 0),
  quantColor(4, 4, 1, 0),
  quantColor(4, 4, 2, 0),
  quantColor(4, 4, 3, 0),
  quantColor(4, 4, 4, 0),
  quantColor(4, 3, 4, 0),
  quantColor(4, 2, 4, 0),
  quantColor(4, 1, 4, 0),
  
  quantColor(4, 0, 4, 0),
  quantColor(4, 0, 4, 1),
  quantColor(4, 0, 4, 2),
  quantColor(4, 0, 4, 3),
  quantColor(4, 0, 4, 4),
  quantColor(4, 0, 3, 4),
  quantColor(4, 0, 2, 4),
  quantColor(4, 0, 1, 4),
  
  quantColor(4, 0, 0, 4),
  quantColor(4, 1, 0, 4),
  quantColor(4, 2, 0, 4),
  quantColor(4, 3, 0, 4),
  quantColor(4, 4, 0, 4),
  quantColor(4, 4, 0, 3),
  quantColor(4, 4, 0, 2),
  quantColor(4, 4, 0, 1),
};

constexpr std::array<QRgb, 9> gray_palette = {
  quantGray(8, 0),
  quantGray(8, 1),
  quantGray(8, 2),
  quantGray(8, 3),
  quantGray(8, 4),
  quantGray(8, 5),
  quantGray(8, 6),
  quantGray(8, 7),
  
  quantGray(8, 8),
};

}

void Palette::initDefault() {
  auto iter = colors.begin();
  switch (canvasFormat) {
    case Format::palette:
      *iter++ = 0;
      *iter++ = qRgb(0, 0, 0);
      *iter++ = qRgb(255, 255, 255);
      break;
    case Format::rgba:
      iter = std::copy(hue_palette.cbegin(), hue_palette.cend(), iter);
    case Format::gray:
      iter = std::copy(gray_palette.cbegin(), gray_palette.cend(), iter);
  }
  std::fill(iter, colors.end(), 0);
  Q_EMIT paletteChanged(PaletteSpan{colors.data()});
}

void Palette::serialize(QIODevice *dev) const {
  assert(dev);
  dev->write(
    reinterpret_cast<const char *>(colors.data()),
    pal_colors * sizeof(QRgb)
  );
}

void Palette::deserialize(QIODevice *dev) {
  assert(dev);
  dev->read(
    reinterpret_cast<char *>(colors.data()),
    pal_colors * sizeof(QRgb)
  );
  Q_EMIT paletteChanged(PaletteSpan{colors.data()});
}

void Palette::initCanvas(const Format format) {
  canvasFormat = format;
}

#include "palette.moc"
