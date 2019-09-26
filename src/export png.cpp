//
//  export png.cpp
//  Animera
//
//  Created by Indi Kernick on 20/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export png.hpp"

#include "png.hpp"
#include <QtCore/qfile.h>
#include "surface factory.hpp"
#include <Graphics/format.hpp>
#include <Graphics/convert.hpp>

namespace {

int getBitDepth(const ExportFormat format) {
  return format == ExportFormat::monochrome ? 1 : 8;
}

int getColorType(const ExportFormat format) {
  switch (format) {
    case ExportFormat::rgba:
      return PNG_COLOR_TYPE_RGB_ALPHA;
    case ExportFormat::index:
      return PNG_COLOR_TYPE_PALETTE;
    case ExportFormat::gray:
      return PNG_COLOR_TYPE_GRAY;
    case ExportFormat::gray_alpha:
      return PNG_COLOR_TYPE_GRAY_ALPHA;
    case ExportFormat::monochrome:
      return PNG_COLOR_TYPE_GRAY;
  }
}

std::vector<png_bytep> getRows(QImage &image) {
  std::vector<png_bytep> rows;
  png_bytep row = image.bits();
  const ptrdiff_t pitch = image.bytesPerLine();
  const ptrdiff_t height = image.height();
  const png_bytep endRow = row + pitch * height;
  rows.reserve(height);
  while (row != endRow) {
    rows.push_back(row);
    row += pitch;
  }
  return rows;
}

void set_PLTE_tRNS(png_structp png, png_infop info, PaletteCSpan palette) {
  png_color plte[pal_colors];
  png_byte trns[pal_colors];
  for (size_t i = 0; i != pal_colors; ++i) {
    const gfx::Color color = gfx::ARGB::color(palette[i]);
    plte[i].red = color.r;
    plte[i].green = color.g;
    plte[i].blue = color.b;
    trns[i] = color.a;
  }
  png_set_PLTE(png, info, plte, pal_colors);
  png_set_tRNS(png, info, trns, pal_colors, nullptr);
}

}

Error exportPng(
  const QString &path,
  const PaletteCSpan palette,
  QImage image,
  const Format canvasFormat,
  const ExportFormat exportFormat
) {
  QString errorMessage;
  png_structp png = png_create_write_struct(
    PNG_LIBPNG_VER_STRING, &errorMessage, &pngError, &pngWarning
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
    return errorMessage;
  }
  QFile file{path};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return "Failed to open file for writing";
  }
  if (setjmp(png_jmpbuf(png))) { // call file dtor
    png_destroy_write_struct(&png, &info);
    return errorMessage;
  }
  png_set_write_fn(png, &file, &pngWrite, &pngFlush);
  png_set_IHDR(
    png,
    info,
    image.width(),
    image.height(),
    getBitDepth(exportFormat),
    getColorType(exportFormat),
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  // TODO: avoid doing a bunch of allocations for each image
  // allocate one image at the very beginning of the export and reuse it
  // the rows vector could also get the same treatment
  // would this be as simple as making the variable static?
  int transforms = PNG_TRANSFORM_IDENTITY;
  switch (exportFormat) {
    case ExportFormat::rgba:
      // TODO: Make ARGB endian aware so that we don't need to do this
      transforms = PNG_TRANSFORM_BGR;
      break;
    case ExportFormat::index:
      set_PLTE_tRNS(png, info, palette);
      break;
    case ExportFormat::gray:
      if (canvasFormat == Format::gray) {
        gfx::convertInplace(makeSurface<gfx::YA::Pixel>(image), gfx::Y{}, gfx::YA{});
      }
      break;
    case ExportFormat::gray_alpha:
      break;
    case ExportFormat::monochrome:
      if (canvasFormat == Format::gray) {
        gfx::convertToMono<gfx::YA, 128>(makeSurface<gfx::YA::Pixel>(image));
      } else if (canvasFormat == Format::index) {
        gfx::convertToMono<gfx::Y, 1>(makeSurface<gfx::Y::Pixel>(image));
      } else Q_UNREACHABLE();
      break;
  }
  std::vector<png_bytep> rows = getRows(image);
  if (setjmp(png_jmpbuf(png))) { // call rows dtor
    png_destroy_write_struct(&png, &info);
    return errorMessage;
  }
  png_set_rows(png, info, rows.data());
  // TODO: png_write_row maybe?
  png_write_png(png, info, transforms, nullptr);
  
  /*
  rgba
    rgba
  index
    rgba
    index (not composited)
    gray (not composited)
    monochrome (not composited)
  gray
    gray
    gray_alpha
    monochrome
  */
  
  png_destroy_write_struct(&png, &info);
  return {};
}
