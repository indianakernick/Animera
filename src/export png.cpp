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

int getPaletteColorType(const Format format) {
  switch (format) {
    case Format::rgba:
    case Format::index:
      return PNG_COLOR_TYPE_RGB_ALPHA;
    case Format::gray:
      return PNG_COLOR_TYPE_GRAY_ALPHA;
  }
}

Format getFormat(const int colorType) {
  switch (colorType) {
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      return Format::gray;
    case PNG_COLOR_TYPE_PALETTE:
      return Format::index;
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      return Format::rgba;
    default: Q_UNREACHABLE();
  }
}

template <typename Func>
void eachRow(QImage &image, Func func) {
  png_bytep row = image.bits();
  const ptrdiff_t pitch = image.bytesPerLine();
  const ptrdiff_t height = image.height();
  const png_bytep endRow = row + pitch * height;
  while (row != endRow) {
    func(row);
    row += pitch;
  }
}

struct WriteContext {
  QString msg;
  QIODevice *dev;
  png_structp png = nullptr;
  png_infop info = nullptr;
};

struct ReadContext {
  QString msg;
  QIODevice *dev;
  png_structp png = nullptr;
  png_infop info = nullptr;
  png_infop endInfo = nullptr;
};

Error initWritePng(WriteContext &ctx) {
  ctx.png = png_create_write_struct(
    PNG_LIBPNG_VER_STRING, &ctx.msg, &pngError, &pngWarning
  );
  if (!ctx.png) {
    // does this only happen when malloc returns nullptr?
    return "Failed to initialize png write struct";
  }
  return {};
}

Error initWriteInfo(WriteContext &ctx) {
  ctx.info = png_create_info_struct(ctx.png);
  if (!ctx.info) {
    png_destroy_write_struct(&ctx.png, nullptr);
    return "Failed to initialize png info struct";
  }
  return {};
}

Error destroyWrite(WriteContext &ctx) {
  png_destroy_write_struct(&ctx.png, &ctx.info);
  return ctx.msg;
}

Error initWriteFile(WriteContext &ctx) {
  if (setjmp(png_jmpbuf(ctx.png))) {
    return destroyWrite(ctx);
  }
  png_set_write_fn(ctx.png, ctx.dev, &pngWrite, &pngFlush);
  return {};
}

Error initWrite(WriteContext &ctx) {
  TRY(initWritePng(ctx));
  TRY(initWriteInfo(ctx));
  TRY(initWriteFile(ctx));
  return {};
}

png_color toPngColor(const gfx::Color color) {
  return {color.r, color.g, color.b};
}

void writePalette(WriteContext &ctx, PaletteCSpan palette) {
  png_color plte[pal_colors];
  png_byte trns[pal_colors];
  
  int i = pal_colors - 1;
  for (; i != -1; --i) {
    if (palette[i] != 0) break;
  }
  if (i == -1) {
    plte[0] = {0, 0, 0};
    trns[0] = 0;
    png_set_PLTE(ctx.png, ctx.info, plte, 1);
    png_set_tRNS(ctx.png, ctx.info, trns, 1, nullptr);
    return;
  }
  
  const int plteSize = i + 1;
  for (; i != -1; --i) {
    const gfx::Color color = gfx::ARGB::color(palette[i]);
    if (color.a != 255) break;
    plte[i] = toPngColor(color);
  }
  
  const int trnsSize = i + 1;
  for (; i != -1; --i) {
    const gfx::Color color = gfx::ARGB::color(palette[i]);
    plte[i] = toPngColor(color);
    trns[i] = color.a;
  }
  
  png_set_PLTE(ctx.png, ctx.info, plte, plteSize);
  if (trnsSize != 0) {
    png_set_tRNS(ctx.png, ctx.info, trns, trnsSize, nullptr);
  }
}

Error initReadPng(ReadContext &ctx) {
  ctx.png = png_create_read_struct(
    PNG_LIBPNG_VER_STRING, &ctx.msg, &pngError, &pngWarning
  );
  if (!ctx.png) {
    // does this only happen when malloc returns nullptr?
    return "Failed to initialize png read struct";
  }
  return {};
}

Error initReadInfo(ReadContext &ctx) {
  ctx.info = png_create_info_struct(ctx.png);
  if (!ctx.info) {
    png_destroy_read_struct(&ctx.png, nullptr, nullptr);
    return "Failed to initialize png info struct";
  }
  ctx.endInfo = png_create_info_struct(ctx.png);
  if (!ctx.endInfo) {
    png_destroy_read_struct(&ctx.png, &ctx.info, nullptr);
    return "Failed to initialize png end info struct";
  }
  return {};
}

Error destroyRead(ReadContext &ctx) {
  png_destroy_read_struct(&ctx.png, &ctx.info, &ctx.endInfo);
  return ctx.msg;
}

Error initReadFile(ReadContext &ctx) {
  if (setjmp(png_jmpbuf(ctx.png))) {
    return destroyRead(ctx);
  }
  png_set_read_fn(ctx.png, ctx.dev, &pngRead);
  return {};
}

Error initRead(ReadContext &ctx) {
  TRY(initReadPng(ctx));
  TRY(initReadInfo(ctx));
  TRY(initReadFile(ctx));
  return {};
}

void readPalette(ReadContext &ctx, PaletteSpan palette) {
  png_color *plte;
  int plteSize;
  if (png_get_PLTE(ctx.png, ctx.info, &plte, &plteSize) != PNG_INFO_PLTE) {
    plteSize = 0;
  }
  png_byte *trns;
  int trnsSize;
  if (png_get_tRNS(ctx.png, ctx.info, &trns, &trnsSize, nullptr) != PNG_INFO_tRNS) {
    trnsSize = 0;
  }
  size_t i = 0;
  for (; i != static_cast<size_t>(trnsSize); ++i) {
    palette[i] = gfx::ARGB::pixel(plte[i].red, plte[i].green, plte[i].blue, trns[i]);
  }
  for (; i != static_cast<size_t>(plteSize); ++i) {
    palette[i] = gfx::ARGB::pixel(plte[i].red, plte[i].green, plte[i].blue);
  }
  for (; i != pal_colors; ++i) {
    palette[i] = 0;
  }
}

void fillRows(
  png_bytepp rowPtr,
  png_bytep row,
  const ptrdiff_t pitch,
  const size_t height
) {
  const png_bytep endRow = row + pitch * height;
  while (row != endRow) {
    *rowPtr = row;
    ++rowPtr;
    row += pitch;
  }
}

}

Error exportPng(
  QIODevice &dev,
  const PaletteCSpan palette,
  QImage image,
  const Format canvasFormat,
  const ExportFormat exportFormat
) {
  WriteContext ctx;
  ctx.dev = &dev;
  TRY(initWrite(ctx));
  
  if (setjmp(png_jmpbuf(ctx.png))) {
    return destroyWrite(ctx);
  }
  
  png_set_IHDR(
    ctx.png,
    ctx.info,
    image.width(),
    image.height(),
    getBitDepth(exportFormat),
    getColorType(exportFormat),
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  
  switch (exportFormat) {
    case ExportFormat::rgba:
      // TODO: Make ARGB endian aware so that we don't need to do this
      png_set_bgr(ctx.png);
      break;
    case ExportFormat::index:
      writePalette(ctx, palette);
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
  
  png_write_info(ctx.png, ctx.info);
  eachRow(image, [&ctx](png_bytep row) {
    png_write_row(ctx.png, row);
  });
  png_write_end(ctx.png, ctx.info);
  
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
  
  return destroyWrite(ctx);
}

Error importPng(
  QIODevice &dev,
  const PaletteSpan palette,
  QImage &image,
  Format &format
) {
  ReadContext ctx;
  ctx.dev = &dev;
  TRY(initRead(ctx));
  std::unique_ptr<png_bytep[]> rows;
  
  if (setjmp(png_jmpbuf(ctx.png))) {
    return destroyRead(ctx);
  }
  
  png_read_info(ctx.png, ctx.info);
  png_uint_32 width, height;
  int colorType;
  png_get_IHDR(ctx.png, ctx.info, &width, &height, 0, &colorType, 0, 0, 0);
  png_set_interlace_handling(ctx.png);
  png_set_strip_16(ctx.png);
  png_set_packing(ctx.png);
  if (!(colorType & PNG_COLOR_MASK_PALETTE) && !(colorType & PNG_COLOR_MASK_ALPHA)) {
    png_set_filler(ctx.png, 255, PNG_FILLER_AFTER);
  }
  png_set_bgr(ctx.png);
  png_read_update_info(ctx.png, ctx.info);
  
  format = getFormat(colorType);
  if (format == Format::index) {
    readPalette(ctx, palette);
  }
  image = {static_cast<int>(width), static_cast<int>(height), qimageFormat(format)};
  
  rows = std::make_unique<png_bytep[]>(height);
  fillRows(rows.get(), image.bits(), image.bytesPerLine(), height);
  png_read_image(ctx.png, rows.get());
  
  png_read_end(ctx.png, ctx.info);
  
  return destroyRead(ctx);
}

Error exportPng(
  QIODevice &dev,
  const PaletteCSpan palette,
  const Format format
) {
  WriteContext ctx;
  ctx.dev = &dev;
  TRY(initWrite(ctx));
  
  if (setjmp(png_jmpbuf(ctx.png))) {
    return destroyWrite(ctx);
  }
  
  png_set_IHDR(
    ctx.png,
    ctx.info,
    static_cast<int>(palette.size()),
    1,
    8,
    getPaletteColorType(format),
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  
  png_write_info(ctx.png, ctx.info);
  
  if (format == Format::gray) {
    gfx::YA::Pixel rowDat[pal_colors];
    std::copy(palette.begin(), palette.end(), rowDat);
    png_write_row(ctx.png, reinterpret_cast<png_bytep>(rowDat));
  } else if (format == Format::rgba || format == Format::index) {
    png_set_bgr(ctx.png);
    png_write_row(ctx.png, reinterpret_cast<png_bytep>(
      const_cast<QRgb *>(palette.data())
    ));
  }
  
  png_write_end(ctx.png, ctx.info);
  
  return destroyWrite(ctx);
}

Error importPng(
  QIODevice &dev,
  const PaletteSpan palette,
  const Format format
) {
  ReadContext ctx;
  ctx.dev = &dev;
  TRY(initRead(ctx));
  std::unique_ptr<png_byte[]> imageData;
  std::unique_ptr<png_bytep[]> rows;
  
  if (setjmp(png_jmpbuf(ctx.png))) {
    return destroyRead(ctx);
  }
  
  png_read_info(ctx.png, ctx.info);
  png_uint_32 width, height;
  int colorType;
  png_get_IHDR(ctx.png, ctx.info, &width, &height, 0, &colorType, 0, 0, 0);
  png_set_interlace_handling(ctx.png);
  png_set_strip_16(ctx.png);
  png_set_expand(ctx.png);
  if (colorType & PNG_COLOR_MASK_COLOR && format == Format::gray) {
    png_set_rgb_to_gray(ctx.png, PNG_ERROR_ACTION_NONE, PNG_RGB_TO_GRAY_DEFAULT, PNG_RGB_TO_GRAY_DEFAULT);
  } else if (!(colorType & PNG_COLOR_MASK_COLOR) && format != Format::gray) {
    png_set_gray_to_rgb(ctx.png);
  }
  if (!(colorType & PNG_COLOR_MASK_ALPHA)) {
    png_set_filler(ctx.png, 255, PNG_FILLER_AFTER);
  }
  png_set_bgr(ctx.png);
  png_read_update_info(ctx.png, ctx.info);
  
  const size_t pitch = png_get_rowbytes(ctx.png, ctx.info);
  imageData = std::make_unique<png_byte[]>(pitch * height);
  rows = std::make_unique<png_bytep[]>(height);
  fillRows(rows.get(), imageData.get(), pitch, height);
  png_read_image(ctx.png, rows.get());
  
  const size_t length = std::min(
    static_cast<size_t>(palette.size()),
    size_t{width} * height
  );
  
  auto copy = [palette, length, &imageData](auto bytes) {
    auto dst = palette.begin();
    const auto dstEnd = dst + length;
    png_bytep src = imageData.get();
    while (dst != dstEnd) {
      std::memcpy(&(*dst), src, bytes);
      src += bytes;
      ++dst;
    }
    std::fill(dst, palette.end(), 0);
  };
  
  if (format == Format::gray) {
    copy(std::integral_constant<size_t, 2>{});
  } else if (format == Format::rgba || format == Format::index) {
    copy(std::integral_constant<size_t, 4>{});
  }
  
  png_read_end(ctx.png, ctx.info);
  
  return destroyRead(ctx);
}
