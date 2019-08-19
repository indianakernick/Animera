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
  return FormatYA::toPixel(quantColor(size, y), 255);
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
    colors[i] = FormatARGB::toPixel(color);
  }
  for (size_t i = plteSize; i != pal_colors; ++i) {
    colors[i] = 0;
  }
}*/

size_t getUsedSize(const PaletteCSpan colors) {
  for (size_t i = static_cast<size_t>(colors.size()); i != 0; --i) {
    if (colors[i - 1] != 0) {
      return i;
    }
  }
  return 0;
}

Error writeRgba(QIODevice &dev, const PaletteCSpan colors) try {
  const size_t used = getUsedSize(colors);
  ChunkWriter writer{dev};
  writer.begin(static_cast<uint32_t>(used) * 4, chunk_palette);
  for (size_t i = 0; i != used; ++i) {
    const Color color = FormatARGB::toColor(colors[i]);
    writer.writeByte(color.r);
    writer.writeByte(color.g);
    writer.writeByte(color.b);
    writer.writeByte(color.a);
  }
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error writeGray(QIODevice &dev, const PaletteCSpan colors) try {
  const size_t used = getUsedSize(colors);
  ChunkWriter writer{dev};
  writer.begin(static_cast<uint32_t>(used) * 2, chunk_palette);
  for (size_t i = 0; i != used; ++i) {
    const Color color = FormatYA::toColor(colors[i]);
    writer.writeByte(color.r);
    writer.writeByte(color.a);
  };
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error checkStart(ChunkStart start, const int multiple) {
  if (Error err = expectedName(start, chunk_palette); err) return err;
  if (start.length % multiple != 0 || start.length / multiple > pal_colors) {
    QString msg = "Invalid ";
    msg += chunk_palette;
    msg += " chunk length ";
    msg += QString::number(start.length);
    return msg;
  }
  return {};
}

Error readRgba(QIODevice &dev, const PaletteSpan colors) try {
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  if (Error err = checkStart(start, 4); err) return err;
  auto iter = colors.begin();
  const auto end = colors.begin() + start.length / 4;
  for (; iter != end; ++iter) {
    Color color;
    color.r = reader.readByte();
    color.g = reader.readByte();
    color.b = reader.readByte();
    color.a = reader.readByte();
    *iter = FormatARGB::toPixel(color);
  }
  if (Error err = reader.end(); err) return err;
  std::fill(iter, colors.end(), 0);
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readGray(QIODevice &dev, const PaletteSpan colors) try {
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  if (Error err = checkStart(start, 2); err) return err;
  auto iter = colors.begin();
  const auto end = colors.begin() + start.length / 2;
  for (; iter != end; ++iter) {
    Color color;
    color.r = reader.readByte();
    color.a = reader.readByte();
    *iter = FormatYA::toPixel(color);
  }
  if (Error err = reader.end(); err) return err;
  std::fill(iter, colors.end(), 0);
  return {};
} catch (FileIOError &e) {
  return e.what();
}

}

Error Palette::serialize(QIODevice &dev) const {
  switch (canvasFormat) {
    case Format::rgba:
    case Format::index:
      return writeRgba(dev, colors);
    case Format::gray:
      return writeGray(dev, colors);
  }
}

Error Palette::deserialize(QIODevice &dev) try {
  switch (canvasFormat) {
    case Format::rgba:
    case Format::index:
      if (Error err = readRgba(dev, colors); err) return err;
      break;
    case Format::gray:
      if (Error err = readGray(dev, colors); err) return err;
      break;
  }
  Q_EMIT paletteChanged({colors.data(), pal_colors});
  return {};
} catch (FileIOError &e) {
  return e.what();
}

PaletteCSpan Palette::getPalette() const {
  return {colors.data(), pal_colors};
}

void Palette::initCanvas(const Format format) {
  canvasFormat = format;
}

#include "palette.moc"
