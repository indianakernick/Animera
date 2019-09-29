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

struct WriteContext {
  QString msg;
  QFile file;
  png_structp png = nullptr;
  png_infop info = nullptr;
};

struct ReadContext {
  QString msg;
  QFile file;
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

Error initWriteFile(WriteContext &ctx, const QString &path) {
  ctx.file.setFileName(path);
  if (!ctx.file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return "Failed to open file for writing";
  }
  if (setjmp(png_jmpbuf(ctx.png))) {
    return destroyWrite(ctx);
  }
  png_set_write_fn(ctx.png, &ctx.file, &pngWrite, &pngFlush);
  return {};
}

Error initWrite(WriteContext &ctx, const QString &path) {
  if (Error err = initWritePng(ctx)) return err;
  if (Error err = initWriteInfo(ctx)) return err;
  if (Error err = initWriteFile(ctx, path)) return err;
  return {};
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

Error initReadFile(ReadContext &ctx, const QString &path) {
  ctx.file.setFileName(path);
  if (!ctx.file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly)) {
    return "Failed to open file for reading";
  }
  if (setjmp(png_jmpbuf(ctx.png))) {
    return destroyRead(ctx);
  }
  png_set_read_fn(ctx.png, &ctx.file, &pngRead);
  return {};
}

Error initRead(ReadContext &ctx, const QString &path) {
  if (Error err = initReadPng(ctx)) return err;
  if (Error err = initReadInfo(ctx)) return err;
  if (Error err = initReadFile(ctx, path)) return err;
  return {};
}

}

Error exportPng(
  const QString &path,
  const PaletteCSpan palette,
  QImage image,
  const Format canvasFormat,
  const ExportFormat exportFormat
) {
  WriteContext ctx;
  if (Error err = initWrite(ctx, path)) return err;
  
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
  // TODO: avoid doing a bunch of allocations for each image
  // allocate one image at the very beginning of the export and reuse it
  // would this be as simple as making the variable static?
  switch (exportFormat) {
    case ExportFormat::rgba:
      // TODO: Make ARGB endian aware so that we don't need to do this
      png_set_bgr(ctx.png);
      break;
    case ExportFormat::index:
      set_PLTE_tRNS(ctx.png, ctx.info, palette);
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

Error exportPng(const QString &path, const PaletteCSpan palette, const Format format) {
  WriteContext ctx;
  if (Error err = initWrite(ctx, path)) return err;
  
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

Error importPng(const QString &path, const PaletteSpan palette, const Format format) {
  ReadContext ctx;
  if (Error err = initRead(ctx, path)) return err;
  
  if (setjmp(png_jmpbuf(ctx.png))) {
    return destroyRead(ctx);
  }
  
  png_read_png(ctx.png, ctx.info, PNG_TRANSFORM_BGR | PNG_TRANSFORM_EXPAND, nullptr);
  const png_byte channels = png_get_channels(ctx.png, ctx.info);
  const size_t width = png_get_image_width(ctx.png, ctx.info);
  const size_t height = png_get_image_height(ctx.png, ctx.info);
  const size_t length = std::min(
    static_cast<size_t>(palette.size()),
    width * height
  );
  
  auto copy = [&ctx, palette, width, length](auto bytes) {
    auto dst = palette.begin();
    const auto dstEnd = dst + length;
    png_bytepp rowIter = png_get_rows(ctx.png, ctx.info);;
    png_bytep colIter = *rowIter;
    png_bytep colEnd = colIter + width * bytes;
    
    while (dst != dstEnd) {
      if constexpr (bytes == 1) {
        *dst = gfx::YA::pixel(*colIter, 255);
      } else if constexpr (bytes == 3) {
        *dst = gfx::ARGB::pixel(colIter[2], colIter[1], colIter[0], 255);
      } else {
        std::memcpy(&(*dst), colIter, bytes);
      }
      colIter += bytes;
      if (colIter == colEnd) {
        ++rowIter;
        colIter = *rowIter;
        colEnd = colIter + width * bytes;
      }
      ++dst;
    }
    std::fill(dst, palette.end(), 0);
  };
  
  if (format == Format::gray) {
    /*png_set_expand_gray_1_2_4_to_8(ctx.png);
    png_set_add_alpha(ctx.png, 255, PNG_FILLER_BEFORE);*/
    if (channels == 1) {
      copy(std::integral_constant<size_t, 1>{});
    } else if (channels == 2) {
      copy(std::integral_constant<size_t, 2>{});
    } else {
      if (Error err = destroyRead(ctx); err) return err;
      return "Expected grayscale image";
    }
  } else if (format == Format::rgba || format == Format::index) {
    /*png_set_add_alpha(ctx.png, 255, PNG_FILLER_BEFORE);
    png_set_bgr(ctx.png);
    png_set_expand(ctx.png);
    png_set_palette_to_rgb(ctx.png);
    png_set_tRNS_to_alpha(ctx.png);
    png_set_gray_to_rgb(ctx.png);*/
    if (channels == 3) {
      copy(std::integral_constant<size_t, 3>{});
    } else if (channels == 4) {
      copy(std::integral_constant<size_t, 4>{});
    } else {
      if (Error err = destroyRead(ctx); err) return err;
      return "Expected color image";
    }
  }
  
  return destroyRead(ctx);
}
