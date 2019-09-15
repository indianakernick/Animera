//
//  sprite file.cpp
//  Animera
//
//  Created by Indi Kernick on 31/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "sprite file.hpp"

#include "zlib.hpp"
#include "serial.hpp"
#include "cell span.hpp"
#include <Graphics/format.hpp>

namespace {

struct DeflateDeleter {
  void operator()(z_streamp stream) const noexcept {
    assertEval(deflateEnd(stream) == Z_OK);
  }
};

struct InflateDeleter {
  void operator()(z_streamp stream) const noexcept {
    assertEval(inflateEnd(stream) == Z_OK);
  }
};

constexpr int byteDepth(const Format format) {
  switch (format) {
    case Format::rgba:
      return 4;
    case Format::index:
      return 1;
    case Format::gray:
      return 2;
  }
}

constexpr uint8_t formatByte(const Format format) {
  return byteDepth(format);
}

void copyToByteOrder(
  unsigned char *dst,
  const unsigned char *src,
  const size_t size,
  const Format format
) {
  assert(size % byteDepth(format) == 0);
  switch (format) {
    case Format::rgba: {
      const auto *srcPx = reinterpret_cast<const gfx::ARGB::Pixel *>(src);
      unsigned char *dstEnd = dst + size;
      while (dst != dstEnd) {
        const gfx::Color color = gfx::ARGB::color(*srcPx++);
        *dst++ = color.r;
        *dst++ = color.g;
        *dst++ = color.b;
        *dst++ = color.a;
      }
      break;
    }
    case Format::index:
      static_assert(sizeof(gfx::I<>::Pixel) == 1);
      std::memcpy(dst, src, size);
      break;
    case Format::gray: {
      auto *srcPx = reinterpret_cast<const gfx::YA::Pixel *>(src);
      unsigned char *dstEnd = dst + size;
      while (dst != dstEnd) {
        const gfx::Color color = gfx::YA::color(*srcPx++);
        *dst++ = color.r;
        *dst++ = color.a;
      }
      break;
    }
  }
}

void copyFromByteOrder(
  unsigned char *dst,
  const unsigned char *src,
  const size_t size,
  const Format format
) {
  assert(size % byteDepth(format) == 0);
  switch (format) {
    case Format::rgba: {
      auto *dstPx = reinterpret_cast<gfx::ARGB::Pixel *>(dst);
      const unsigned char *srcEnd = src + size;
      while (src != srcEnd) {
        gfx::Color color;
        color.r = *src++;
        color.g = *src++;
        color.b = *src++;
        color.a = *src++;
        *dstPx++ = gfx::ARGB::pixel(color);
      }
      break;
    }
    case Format::index:
      static_assert(sizeof(gfx::I<>::Pixel) == 1);
      std::memcpy(dst, src, size);
      break;
    case Format::gray: {
      auto *dstPx = reinterpret_cast<gfx::YA::Pixel *>(dst);
      const unsigned char *srcEnd = src + size;
      while (src != srcEnd) {
        gfx::Color color;
        color.r = *src++;
        color.a = *src++;
        *dstPx = gfx::YA::pixel(color);
      }
      break;
    }
  }
}

size_t getUsedSize(const PaletteCSpan colors) {
  for (size_t i = colors.size(); i != 0; --i) {
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
    const gfx::Color color = gfx::ARGB::color(colors[i]);
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
    const gfx::Color color = gfx::YA::color(colors[i]);
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
    gfx::Color color;
    color.r = reader.readByte();
    color.g = reader.readByte();
    color.b = reader.readByte();
    color.a = reader.readByte();
    *iter = gfx::ARGB::pixel(color);
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
    gfx::Color color;
    color.r = reader.readByte();
    color.a = reader.readByte();
    *iter = gfx::YA::pixel(color);
  }
  if (Error err = reader.end(); err) return err;
  std::fill(iter, colors.end(), 0);
  return {};
} catch (FileIOError &e) {
  return e.what();
}

}

Error writeAHDR(QIODevice &dev, const SpriteInfo &info) try {
  ChunkWriter writer{dev};
  writer.begin(5 * file_int_size + 1, chunk_anim_header);
  writer.writeInt(info.width);
  writer.writeInt(info.height);
  writer.writeInt(static_cast<uint32_t>(info.layers));
  writer.writeInt(static_cast<uint32_t>(info.frames));
  writer.writeInt(info.delay);
  writer.writeByte(formatByte(info.format));
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error writePLTE(QIODevice &dev, const PaletteCSpan colors, const Format format) try {
  switch (format) {
    case Format::rgba:
    case Format::index:
      return writeRgba(dev, colors);
    case Format::gray:
      return writeGray(dev, colors);
  }
} catch (FileIOError &e) {
  return e.what();
}

Error writeLHDR(QIODevice &dev, const Layer &layer) try {
  ChunkWriter writer{dev};
  const uint32_t nameLen = static_cast<uint32_t>(layer.name.size());
  writer.begin(file_int_size + 1 + nameLen, chunk_layer_header);
  writer.writeInt(static_cast<uint32_t>(layer.spans.size()));
  writer.writeByte(layer.visible);
  writer.writeString(layer.name.data(), nameLen);
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error writeCHDR(QIODevice &dev, const CellSpan &span) try {
  ChunkWriter writer{dev};
  writer.begin(chunk_cell_header);
  writer.writeInt(static_cast<uint32_t>(span.len));
  if (*span.cell) {
    writer.writeInt(span.cell->image.offset().x());
    writer.writeInt(span.cell->image.offset().y());
    writer.writeInt(span.cell->image.width());
    writer.writeInt(span.cell->image.height());
  }
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error writeCDAT(QIODevice &dev, const QImage &image, const Format format) try {
  assert(!image.isNull());
  const uint32_t outBuffSize = file_buff_size;
  std::vector<Bytef> outBuff(outBuffSize);
  const uint32_t inBuffSize = image.width() * byteDepth(format);
  std::vector<Bytef> inBuff(inBuffSize);
  
  // @TODO avoid overflowing uint32_t
  // might need to reduce maximum image size
  // or split the image data into multiple chunks
  
  z_stream stream;
  stream.zalloc = nullptr;
  stream.zfree = nullptr;
  int ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
  if (ret == Z_MEM_ERROR) return "zlib: memory error";
  assert(ret == Z_OK);
  const std::unique_ptr<z_stream, DeflateDeleter> deleter{&stream};
  stream.avail_in = 0;
  stream.next_out = outBuff.data();
  stream.avail_out = outBuffSize;
  
  ChunkWriter writer{dev};
  writer.begin(chunk_cell_data);
  
  int rowIdx = 0;
  
  do {
    if (stream.avail_in == 0 && rowIdx < image.height()) {
      copyToByteOrder(inBuff.data(), image.scanLine(rowIdx), inBuffSize, format);
      stream.next_in = inBuff.data();
      stream.avail_in = inBuffSize;
      ++rowIdx;
    }
    if (stream.avail_out == 0) {
      writer.writeString(outBuff.data(), outBuffSize);
      stream.next_out = outBuff.data();
      stream.avail_out = outBuffSize;
    }
    ret = deflate(&stream, stream.avail_in ? Z_NO_FLUSH : Z_FINISH);
  } while (ret == Z_OK);
  assert(ret == Z_STREAM_END);
  
  if (stream.avail_out < outBuffSize) {
    writer.writeString(outBuff.data(), outBuffSize - stream.avail_out);
  }
  
  writer.end();
  
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error writeAEND(QIODevice &dev) try {
  ChunkWriter writer{dev};
  writer.begin(0, chunk_anim_end);
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readAHDR(QIODevice &dev, SpriteInfo &info) try {
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  if (Error err = expectedName(start, chunk_anim_header); err) return err;
  if (Error err = expectedLength(start, 5 * file_int_size + 1); err) return err;
  info.width = reader.readInt();
  if (info.width <= 0) return "Negative canvas width";
  info.height = reader.readInt();
  if (info.height <= 0) return "Negative canvas height";
  info.layers = static_cast<LayerIdx>(reader.readInt());
  info.frames = static_cast<FrameIdx>(reader.readInt());
  if (+info.frames < 0) return "Negative frame count";
  info.delay = reader.readInt();
  const uint8_t readFormat = reader.readByte();
  switch (readFormat) {
    case formatByte(Format::rgba):
      info.format = Format::rgba;
      break;
    case formatByte(Format::index):
      info.format = Format::index;
      break;
    case formatByte(Format::gray):
      info.format = Format::gray;
      break;
    default:
      return "Invalid canvas format " + QString::number(readFormat);
  }
  if (Error err = reader.end(); err) return err;
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readPLTE(QIODevice &dev, const PaletteSpan colors, const Format format) try {
  switch (format) {
    case Format::rgba:
    case Format::index:
      if (Error err = readRgba(dev, colors); err) return err;
      break;
    case Format::gray:
      if (Error err = readGray(dev, colors); err) return err;
      break;
  }
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readLHDR(QIODevice &dev, Layer &layer) try {
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  if (Error err = expectedName(start, chunk_layer_header); err) return err;
  if (start.length <= file_int_size + 1) {
    return QString{chunk_layer_header} + " chunk length too small";
  }
  layer.spans.resize(reader.readInt());
  const uint8_t visibleByte = reader.readByte();
  switch (visibleByte) {
    case 0:
      layer.visible = false;
      break;
    case 1:
      layer.visible = true;
      break;
    default:
      return "Invalid visibility " + QString::number(visibleByte);
  }
  const uint32_t nameLen = start.length - (file_int_size + 1);
  if (nameLen > layer_name_max_len) {
    return QString{chunk_layer_header} + " chunk length too big";
  }
  layer.name.resize(nameLen);
  reader.readString(layer.name.data(), nameLen);
  if (Error err = reader.end(); err) return err;
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readCHDR(QIODevice &dev, CellSpan &span, const Format format) try {
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  if (Error err = expectedName(start, chunk_cell_header); err) return err;
  if (start.length != file_int_size && start.length != 5 * file_int_size) {
    return QString{chunk_cell_header} + " chunk length invalid";
  }
  span.len = static_cast<FrameIdx>(reader.readInt());
  if (+span.len <= 0) return "Negative cell span length";
  if (start.length == 5 * file_int_size) {
    const int x = reader.readInt();
    const int y = reader.readInt();
    const int width = reader.readInt();
    if (width <= 0) return "Negative cell width";
    const int height = reader.readInt();
    if (height <= 0) return "Negative cell height";
    span.cell = std::make_unique<Cell>();
    span.cell->image = {width, height, qimageFormat(format)};
    span.cell->image.setOffset({x, y});
  }
  if (Error err = reader.end(); err) return err;
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readCDAT(QIODevice &dev, QImage &image, const Format format) try {
  assert(!image.isNull());
  const uint32_t outBuffSize = image.width() * byteDepth(format);
  std::vector<Bytef> outBuff(outBuffSize);
  const uint32_t inBuffSize = file_buff_size;
  std::vector<Bytef> inBuff(inBuffSize);
  
  z_stream stream;
  stream.zalloc = nullptr;
  stream.zfree = nullptr;
  int ret = inflateInit(&stream);
  if (ret == Z_MEM_ERROR) return "zlib: memory error";
  assert(ret == Z_OK);
  const std::unique_ptr<z_stream, InflateDeleter> deleter{&stream};
  stream.avail_in = 0;
  stream.next_out = outBuff.data();
  stream.avail_out = outBuffSize;
  
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  if (Error err = expectedName(start, chunk_cell_data); err) {
    return err;
  }
  
  int rowIdx = 0;
  uint32_t remainingChunk = start.length;
  
  do {
    if (stream.avail_in == 0 && remainingChunk != 0) {
      stream.next_in = inBuff.data();
      stream.avail_in = std::min(inBuffSize, remainingChunk);
      remainingChunk -= stream.avail_in;
      reader.readString(inBuff.data(), stream.avail_in);
    }
    if (stream.avail_out == 0 && rowIdx < image.height()) {
      copyFromByteOrder(image.scanLine(rowIdx), outBuff.data(), outBuffSize, format);
      stream.next_out = outBuff.data();
      stream.avail_out = outBuffSize;
      ++rowIdx;
    }
    ret = inflate(&stream, Z_NO_FLUSH);
  } while (ret == Z_OK);
  if (ret == Z_DATA_ERROR) {
    return QString{"zlib: "} + stream.msg;
  } else if (ret == Z_MEM_ERROR) {
    return "zlib: memory error";
  }
  assert(ret == Z_STREAM_END);
  
  if (rowIdx == image.height() - 1) {
    if (stream.avail_out != 0) {
      return "Invalid image data";
    }
    copyFromByteOrder(image.scanLine(rowIdx), outBuff.data(), outBuffSize, format);
  } else if (rowIdx == image.height()) {
    if (stream.avail_out != outBuffSize) {
      return "Invalid image data";
    }
  } else {
    return "Invalid image data";
  }
  
  if (Error err = reader.end(); err) return err;
  
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readAEND(QIODevice &dev) try {
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  if (Error err = expectedName(start, chunk_anim_end); err) return err;
  if (Error err = expectedLength(start, 0); err) return err;
  if (Error err = reader.end(); err) return err;
  return {};
} catch (FileIOError &e) {
  return e.what();
}
