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
 - delay
 - format

PLTE
palette
 - colors excluding trailing 0 entries

LHDR
layer header
 - number of spans in the layer
 - byte visibility
 - ascii string name

CHDR
cell header
 - length of span
 - x coord
 - y coord
 - width
 - height

CDAT
cell data
 - compressed image data
 
AEND
animation end
*/

}

Error Sprite::saveFile(const QString &path) const {
  QFile file{path};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return "Failed to open file for writing";
  }
  if (file.write(signature, 8) != 8) {
    return FileIOError{}.what();
  }
  if (Error err = timeline.serializeHead(file); err) return err;
  if (Error err = palette.serialize(file); err) return err;
  if (Error err = timeline.serializeBody(file); err) return err;
  if (Error err = timeline.serializeTail(file); err) return err;
  return {};
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

Error Sprite::openFile(const QString &path) {
  QFile file{path};
  if (!file.open(QIODevice::ReadOnly)) {
    return "Failed to open file for reading";
  }
  char sig[8];
  if (file.read(sig, 8) != 8) {
    return FileIOError{}.what();
  }
  if (std::memcmp(sig, signature, 8) != 0) {
    return "Signature mismatch";
  }
  
  if (Error err = timeline.deserializeHead(file, format, size); err) return err;
  Q_EMIT canvasInitialized(format, size);
  palette.initCanvas(format);
  timeline.initCanvas(format, size);
  if (Error err = palette.deserialize(file); err) return err;
  if (Error err = timeline.deserializeBody(file); err) return err;
  if (Error err = timeline.deserializeTail(file); err) return err;
  
  return {};
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
