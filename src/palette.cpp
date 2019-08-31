//
//  palette.cpp
//  Animera
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "palette.hpp"

#include "serial.hpp"
#include "formats.hpp"
#include "sprite file.hpp"

namespace {

constexpr int quantColor(const double size, const int color) {
  return 255 * color / size + 0.5;
}

constexpr QRgb quantColor(const int size, const int r, const int g, const int b) {
  return qRgb(quantColor(size, r), quantColor(size, g), quantColor(size, b));
}

constexpr QRgb quantGrayColor(const int size, const int y) {
  const int gray = quantColor(size, y);
  return qRgb(gray, gray, gray);
}

constexpr QRgb quantGray(const int size, const int y) {
  return FormatYA::pixel(quantColor(size, y), 255);
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

constexpr std::array<QRgb, 9> gray_color_palette = {
  quantGrayColor(8, 0),
  quantGrayColor(8, 1),
  quantGrayColor(8, 2),
  quantGrayColor(8, 3),
  quantGrayColor(8, 4),
  quantGrayColor(8, 5),
  quantGrayColor(8, 6),
  quantGrayColor(8, 7),
  
  quantGrayColor(8, 8),
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
    case Format::index:
      *iter++ = 0;
      *iter++ = qRgb(0, 0, 0);
      *iter++ = qRgb(255, 255, 255);
      break;
    case Format::rgba:
      iter = std::copy(hue_palette.cbegin(), hue_palette.cend(), iter);
      iter = std::copy(gray_color_palette.cbegin(), gray_color_palette.cend(), iter);
      break;
    case Format::gray:
      iter = std::copy(gray_palette.cbegin(), gray_palette.cend(), iter);
  }
  std::fill(iter, colors.end(), 0);
  Q_EMIT paletteChanged({colors.data(), pal_colors});
}

namespace {

/*void read_PLTE_tRNS(png_structp png, png_infop info, PaletteSpan colors) {
  png_color *plte;
  int plteSize;
  if (png_get_PLTE(png, info, &plte, &plteSize) != PNG_INFO_PLTE) {
    pngError(png, "Missing PLTE chunk");
  }
  png_byte *trns;
  int trnsSize;
  if (png_get_tRNS(png, info, &trns, &trnsSize, nullptr) != PNG_INFO_tRNS) {
    pngError(png, "Missing tRNS chunk");
  }
  if (plteSize != trnsSize) {
    pngError(png, "Differing PLTE and tRNS chunk sizes");
  }
  if (plteSize < 1 || plteSize > pal_colors) {
    pngError(png, "PLTE chunk size out of range");
  }
  for (size_t i = 0; i != static_cast<size_t>(plteSize); ++i) {
    const Color color = {plte[i].red, plte[i].green, plte[i].blue, trns[i]};
    colors[i] = FormatARGB::pixel(color);
  }
  for (size_t i = plteSize; i != pal_colors; ++i) {
    colors[i] = 0;
  }
}*/

}

Error Palette::serialize(QIODevice &dev) const {
  return writePLTE(dev, colors, canvasFormat);
}

Error Palette::deserialize(QIODevice &dev) {
  if (Error err = readPLTE(dev, colors, canvasFormat); err) return err;
  Q_EMIT paletteChanged({colors.data(), pal_colors});
  return {};
}

PaletteCSpan Palette::getPalette() const {
  return {colors.data(), pal_colors};
}

void Palette::initCanvas(const Format format) {
  canvasFormat = format;
}

#include "palette.moc"
