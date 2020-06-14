//
//  palette.cpp
//  Animera
//
//  Created by Indiana Kernick on 7/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "palette.hpp"

#include "file io.hpp"
#include "chunk io.hpp"
#include "export png.hpp"
#include "scope time.hpp"
#include "sprite file.hpp"
#include <Graphics/format.hpp>

namespace {

constexpr int quantColor(const int size, const int color) {
  return scale(color, size, 255);
}

constexpr PixelVar quantColor(const int size, const int r, const int g, const int b) {
  return PixelVar{gfx::ARGB::pixel(quantColor(size, r), quantColor(size, g), quantColor(size, b))};
}

constexpr PixelVar quantGrayColor(const int size, const int y) {
  const int gray = quantColor(size, y);
  return PixelVar{gfx::ARGB::pixel(gray, gray, gray)};
}

constexpr PixelVar quantGray(const int size, const int y) {
  return PixelVar{gfx::YA::pixel(quantColor(size, y), 255)};
}

constexpr std::array<PixelVar, 24> hue_palette = {
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

constexpr std::array<PixelVar, 9> gray_color_palette = {
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

constexpr std::array<PixelVar, 9> gray_palette = {
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

PaletteCSpan getUsedSpan(const PaletteCSpan colors) {
  for (std::size_t i = colors.size(); i != 0; --i) {
    if (!colors[i - 1].zero()) {
      return {colors.data(), i};
    }
  }
  return {};
}

}

void Palette::reset() {
  auto iter = colors.begin();
  switch (canvasFormat) {
    case Format::index:
      *iter++ = PixelVar{gfx::ARGB::pixel(0, 0, 0, 0)};
      *iter++ = PixelVar{gfx::ARGB::pixel(0, 0, 0, 255)};
      *iter++ = PixelVar{gfx::ARGB::pixel(255, 255, 255, 255)};
      break;
    case Format::rgba:
      iter = std::copy(hue_palette.cbegin(), hue_palette.cend(), iter);
      iter = std::copy(gray_color_palette.cbegin(), gray_color_palette.cend(), iter);
      break;
    case Format::gray:
      iter = std::copy(gray_palette.cbegin(), gray_palette.cend(), iter);
  }
  std::fill(iter, colors.end(), PixelVar{});
}

void Palette::initDefault() {
  reset();
  change();
}

Error Palette::serialize(QIODevice &dev) const {
  SCOPE_TIME("Palette::serialize");

  return writePLTE(dev, getUsedSpan(colors), canvasFormat);
}

Error Palette::deserialize(QIODevice &dev) {
  SCOPE_TIME("Palette::deserialize");

  TRY(readPLTE(dev, colors, canvasFormat));
  change();
  return {};
}

Error Palette::save(const QString &path) const {
  FileWriter writer;
  TRY(writer.open(path));
  TRY(exportPalettePng(writer.dev(), getUsedSpan(colors), canvasFormat));
  return writer.flush();
}

Error Palette::open(const QString &path) {
  FileReader reader;
  TRY(reader.open(path));
  TRY(importPalettePng(reader.dev(), colors, canvasFormat));
  return reader.flush();
}

PaletteSpan Palette::getPalette() {
  return {colors.data(), pal_colors};
}

PaletteCSpan Palette::getPalette() const {
  return {colors.data(), pal_colors};
}

void Palette::change() {
  Q_EMIT paletteChanged({colors.data(), pal_colors});
}

void Palette::initCanvas(const Format format) {
  canvasFormat = format;
}

#include "palette.moc"
