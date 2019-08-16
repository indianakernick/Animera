//
//  sprite.cpp
//  Animera
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "sprite.hpp"

#include "png.hpp"
#include "config.hpp"
#include "serial.hpp"
#include <QtCore/qfile.h>
#include "export options.hpp"

void Sprite::newFile(const Format newFormat, const QSize newSize) {
  format = newFormat;
  size = newSize;
  Q_EMIT canvasInitialized(format, size);
  palette.initCanvas(format);
  timeline.initCanvas(format, size);
  palette.initDefault();
  timeline.initDefault();
}

namespace {

constexpr char magic_number[] = {'P', 'I', 'X', '2'};
constexpr char signature[8] = "animera";

int getColorType(const Format format) {
  switch (format) {
    case Format::rgba:  return PNG_COLOR_TYPE_RGBA;
    case Format::index: return PNG_COLOR_TYPE_PALETTE;
    case Format::gray:  return PNG_COLOR_TYPE_GRAY_ALPHA;
    default: Q_UNREACHABLE();
  }
}

/*

AHDR
animation header
 - width
 - height
 - number of layers
 - number of frames
 - maybe delay
 - format

PLTE
palette
 - 256 colors

LHDR
layer header
 - number of spans in the layer
 - byte visibility
 - null-terminated ascii string name

CHDR
cell header
 - length of span
 - x coord
 - y coord
 - width
 - height
 - has data

CDAT
cell data
 - compressed image data
 
AEND
animation end
*/

}

std::optional<QString> Sprite::saveFile(const QString &path) const {
  QFile file{path};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return "Failed to open file for writing";
  }
  if (file.write(signature, 8) != 8) {
    return "File write error";
  }
  if (auto err = timeline.serializeHead(file); err) return err;
  if (auto err = palette.serialize(file); err) return err;
  if (auto err = timeline.serializeBody(file); err) return err;
  if (auto err = timeline.serializeTail(file); err) return err;
  return std::nullopt;
  /*png_structp png = png_create_write_struct(
    PNG_LIBPNG_VER_STRING, nullptr, &pngError, &pngWarning
  );
  if (!png) {
    // does this only happen when malloc returns nullptr?
    return "Failed to initialize png write struct";
  }
  png_infop info = png_create_info_struct(png);
  if (!info) {
    png_destroy_write_struct(&png, nullptr);
    return "Failed to initialize png info struct";
  }
  if (setjmp(png_jmpbuf(png))) {
    png_destroy_write_struct(&png, &info);
    return pngErrorMsg;
  }
  QFile file{path};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return "Failed to open file for writing";
  }
  png_set_write_fn(png, &file, &pngWrite, &pngFlush);
  png_set_IHDR(
    png,
    info,
    size.width(),
    size.height(),
    8,
    getColorType(format),
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  palette.serialize(png, info);
  timeline.serialize(png, info);
  png_destroy_write_struct(&png, &info);
  return std::nullopt;*/
}

std::optional<QString> Sprite::openFile(const QString &path) {
  QFile file{path};
  if (!file.open(QIODevice::ReadOnly)) {
    throw std::exception{};
  }
  char header[sizeof(magic_number)];
  file.read(header, sizeof(magic_number));
  if (std::memcmp(header, magic_number, sizeof(magic_number)) != 0) {
    throw std::exception{};
  }
  deserializeBytes(&file, format);
  size = {
    deserializeBytesAs<uint16_t>(&file),
    deserializeBytesAs<uint16_t>(&file)
  };
  Q_EMIT canvasInitialized(format, size);
  palette.initCanvas(format);
  timeline.initCanvas(format, size);
  palette.deserialize(&file);
  timeline.deserialize(&file);
  return std::nullopt;
}

void Sprite::exportSprite(const ExportOptions &options) const {
  timeline.exportTimeline(options, palette.getPalette());
}

Format Sprite::getFormat() const {
  return format;
}

QSize Sprite::getSize() const {
  return size;
}

#include "sprite.moc"
