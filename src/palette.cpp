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
  return FormatGray::toPixel(quantColor(size, y), 255);
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

/*void write_PLTE_tRNS(png_structp png, png_infop info, PaletteCSpan colors) {
  png_color plte[pal_colors];
  png_byte trns[pal_colors];
  for (size_t i = 0; i != pal_colors; ++i) {
    const Color color = FormatARGB::toColor(colors[i]);
    plte[i].red = color.r;
    plte[i].green = color.g;
    plte[i].blue = color.b;
    trns[i] = color.a;
  }
  png_set_PLTE(png, info, plte, pal_colors);
  png_set_tRNS(png, info, trns, pal_colors, nullptr);
}

void read_PLTE_tRNS(png_structp png, png_infop info, PaletteSpan colors) {
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
    colors[i] = FormatARGB::toPixel(color);
  }
  for (size_t i = plteSize; i != pal_colors; ++i) {
    colors[i] = 0;
  }
}

void writeRgba_apLT(png_structp png, png_infop info, PaletteCSpan colors) {
  Color aplt[pal_colors];
  std::transform(colors.cbegin(), colors.cend(), std::begin(aplt), &FormatARGB::toColor);
  setUnknownChunk(png, info, "apLT", std::span{aplt});
}

void writeGray_apLT(png_structp png, png_infop info, PaletteCSpan colors) {
  struct {
    uint8_t gray;
    uint8_t alpha;
  } aplt[pal_colors];
  for (size_t i = 0; i != pal_colors; ++i) {
    aplt[i].gray = FormatGray::toGray(colors[i]);
    aplt[i].alpha = FormatGray::toAlpha(colors[i]);
  }
  setUnknownChunk(png, info, "apLT", std::span{aplt});
}*/

void writeRgba(QIODevice &dev, const PaletteCSpan colors) {
  ChunkWriter writer{dev};
  writer.begin(pal_colors * sizeof(Color), "PLTE");
  for (const QRgb pixel : colors) {
    const Color color = FormatARGB::toColor(pixel);
    writer.write(color);
  }
  writer.end();
}

void writeGray(QIODevice &dev, const PaletteCSpan colors) {
  struct GrayAlpha {
    uint8_t gray;
    uint8_t alpha;
  };
  
  ChunkWriter writer{dev};
  writer.begin(pal_colors * sizeof(GrayAlpha), "PLTE");
  for (const QRgb pixel : colors) {
    GrayAlpha color;
    color.gray = FormatGray::toGray(pixel);
    color.alpha = FormatGray::toAlpha(pixel);
    writer.write(color);
  };
  writer.end();
}

}

void Palette::serialize(QIODevice &dev) const {
  switch (canvasFormat) {
    case Format::rgba:
    case Format::index:
      return writeRgba(dev, colors);
    case Format::gray:
      return writeGray(dev, colors);
  }
}

void Palette::deserialize(QIODevice *dev) {
  /*if (canvasFormat == Format::index) {
    read_PLTE_tRNS(png, info, colors);
  } else {
    // apLT
    // animation palette
  }
  Q_EMIT paletteChanged({colors.data(), pal_colors});*/
  assert(dev);
  dev->read(
    reinterpret_cast<char *>(colors.data()),
    pal_colors * sizeof(QRgb)
  );
  Q_EMIT paletteChanged({colors.data(), pal_colors});
}

PaletteCSpan Palette::getPalette() const {
  return {colors.data(), pal_colors};
}

void Palette::initCanvas(const Format format) {
  canvasFormat = format;
}

#include "palette.moc"
