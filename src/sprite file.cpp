//
//  sprite file.cpp
//  Animera
//
//  Created by Indi Kernick on 31/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "sprite file.hpp"

#include "zlib.hpp"
#include "strings.hpp"
#include "chunk io.hpp"
#include "cell span.hpp"
#include "scope time.hpp"
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
  SCOPE_TIME("copyToByteOrder");

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
  SCOPE_TIME("copyFromByteOrder");

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
        *dstPx++ = gfx::YA::pixel(color);
      }
      break;
    }
  }
}

}

Error writeSignature(QIODevice &dev) {
  SCOPE_TIME("writeSignature");

  if (dev.write(file_sig, file_sig_len) != file_sig_len) {
    return FileIOError{}.what();
  }
  return {};
}

Error writeAHDR(QIODevice &dev, const SpriteInfo &info) try {
  SCOPE_TIME("writeAHDR");

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

namespace {

Error writeRgba(QIODevice &dev, const PaletteCSpan colors) try {
  ChunkWriter writer{dev};
  writer.begin(static_cast<uint32_t>(colors.size()) * 4, chunk_palette);
  for (size_t i = 0; i != colors.size(); ++i) {
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
  ChunkWriter writer{dev};
  writer.begin(static_cast<uint32_t>(colors.size()) * 2, chunk_palette);
  for (size_t i = 0; i != colors.size(); ++i) {
    const gfx::Color color = gfx::YA::color(colors[i]);
    writer.writeByte(color.r);
    writer.writeByte(color.a);
  };
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.what();
}

}

Error writePLTE(QIODevice &dev, const PaletteCSpan colors, const Format format) try {
  SCOPE_TIME("writePLTE");

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
  SCOPE_TIME("writeLHDR");

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
  SCOPE_TIME("writeCHDR");

  ChunkWriter writer{dev};
  writer.begin(chunk_cell_header);
  writer.writeInt(static_cast<uint32_t>(span.len));
  if (*span.cell) {
    const QRect rect = span.cell->rect();
    writer.writeInt(rect.x());
    writer.writeInt(rect.y());
    writer.writeInt(rect.width());
    writer.writeInt(rect.height());
  }
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error writeCDAT(QIODevice &dev, const QImage &image, const Format format) try {
  SCOPE_TIME("writeCDAT");
  
  static_assert(sizeof(uInt) >= sizeof(uint32_t));
  
  assert(!image.isNull());
  const uint32_t outBuffSize = file_buff_size;
  std::vector<Bytef> outBuff(outBuffSize);
  const uint32_t inBuffSize = image.width() * byteDepth(format);
  std::vector<Bytef> inBuff(inBuffSize);
  
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
  uint32_t remainingChunk = ~uint32_t{};
  
  do {
    if (stream.avail_in == 0 && rowIdx < image.height()) {
      copyToByteOrder(inBuff.data(), image.scanLine(rowIdx), inBuffSize, format);
      stream.next_in = inBuff.data();
      stream.avail_in = inBuffSize;
      ++rowIdx;
    }
    if (stream.avail_out == 0) {
      // This situation is near impossible. This might as well be an assert
      if (remainingChunk < outBuffSize) return "Chunk overflow";
      writer.writeString(outBuff.data(), outBuffSize);
      remainingChunk -= outBuffSize;
      stream.next_out = outBuff.data();
      stream.avail_out = outBuffSize;
    }
    
    SCOPE_TIME("deflate");
    
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
  SCOPE_TIME("writeAEND");

  ChunkWriter writer{dev};
  writer.begin(0, chunk_anim_end);
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readSignature(QIODevice &dev) {
  SCOPE_TIME("readSignature");

  char signature[file_sig_len];
  if (dev.read(signature, file_sig_len) != file_sig_len) {
    return FileIOError{}.what();
  }
  if (std::memcmp(signature, file_sig, file_sig_len) != 0) {
    return "Signature mismatch";
  }
  return {};
}

namespace {

Error readFormatByte(Format &format, const uint8_t byte) {
  switch (byte) {
    case formatByte(Format::rgba):
      format = Format::rgba;
      break;
    case formatByte(Format::index):
      format = Format::index;
      break;
    case formatByte(Format::gray):
      format = Format::gray;
      break;
    default:
      return "Invalid canvas format " + QString::number(byte);
  }
  return {};
}

}

Error readAHDR(QIODevice &dev, SpriteInfo &info) try {
  SCOPE_TIME("readAHDR");

  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_anim_header));
  TRY(expectedLength(start, 5 * file_int_size + 1));
  
  info.width = reader.readInt();
  info.height = reader.readInt();
  info.layers = static_cast<LayerIdx>(reader.readInt());
  info.frames = static_cast<FrameIdx>(reader.readInt());
  info.delay = reader.readInt();
  const uint8_t format = reader.readByte();
  
  TRY(reader.end());
  
  if (info.width <= 0 || max_image_width < info.width) {
    return "Canvas width is out-of-range";
  }
  if (info.height <= 0 || max_image_height < info.height) {
    return "Canvas height is out-of-range";
  }
  // TODO: upper bound for layers and frames?
  if (+info.layers < 0) return "Layer count is out-of-range";
  if (+info.frames < 0) return "Frame count is out-of-range";
  if (info.delay < ctrl_delay.min || ctrl_delay.max < info.delay) {
    return "Animation delay is out-of-range";
  }
  TRY(readFormatByte(info.format, format));
  
  return {};
} catch (FileIOError &e) {
  return e.what();
}

namespace {

Error checkPaletteStart(ChunkStart start, const int multiple) {
  TRY(expectedName(start, chunk_palette));
  if (start.length % multiple != 0 || start.length / multiple > pal_colors) {
    QString msg = "Invalid '";
    msg += toLatinString(chunk_palette);
    msg += "' chunk length ";
    msg += QString::number(start.length);
    return msg;
  }
  return {};
}

Error readRgba(QIODevice &dev, const PaletteSpan colors) try {
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(checkPaletteStart(start, 4));
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
  TRY(reader.end());
  std::fill(iter, colors.end(), 0);
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readGray(QIODevice &dev, const PaletteSpan colors) try {
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(checkPaletteStart(start, 2));
  auto iter = colors.begin();
  const auto end = colors.begin() + start.length / 2;
  for (; iter != end; ++iter) {
    gfx::Color color;
    color.r = reader.readByte();
    color.a = reader.readByte();
    *iter = gfx::YA::pixel(color);
  }
  TRY(reader.end());
  std::fill(iter, colors.end(), 0);
  return {};
} catch (FileIOError &e) {
  return e.what();
}

}

Error readPLTE(QIODevice &dev, const PaletteSpan colors, const Format format) try {
  SCOPE_TIME("readPLTE");
  
  switch (format) {
    case Format::rgba:
    case Format::index:
      TRY(readRgba(dev, colors));
      break;
    case Format::gray:
      TRY(readGray(dev, colors));
      break;
  }
  return {};
} catch (FileIOError &e) {
  return e.what();
}

namespace {

Error readVisibileByte(bool &visible, const uint8_t byte) {
  switch (byte) {
    case 0:
      visible = false;
      break;
    case 1:
      visible = true;
      break;
    default:
      return "Invalid visibility " + QString::number(byte);
  }
  return {};
}

}

Error readLHDR(QIODevice &dev, Layer &layer) try {
  SCOPE_TIME("readLHDR");

  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_layer_header));
  if (start.length <= file_int_size + 1) {
    return "'" + toLatinString(chunk_layer_header) + "' chunk length too small";
  }
  const uint32_t nameLen = start.length - (file_int_size + 1);
  if (nameLen > layer_name_max_len) {
    return "'" + toLatinString(chunk_layer_header) + "' chunk length too big";
  }
  
  const uint32_t spans = reader.readInt();
  const uint8_t visible = reader.readByte();
  
  layer.name.resize(nameLen);
  reader.readString(layer.name.data(), nameLen);
  
  TRY(reader.end());
  
  // TODO: range checking on spans?
  layer.spans.resize(spans);
  TRY(readVisibileByte(layer.visible, visible));
  
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readCHDR(QIODevice &dev, CellSpan &span, const Format format) try {
  SCOPE_TIME("readCHDR");

  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_cell_header));
  if (start.length != file_int_size && start.length != 5 * file_int_size) {
    return "'" + toLatinString(chunk_cell_header) + "' chunk length invalid";
  }
  
  span.len = static_cast<FrameIdx>(reader.readInt());
  QPoint pos;
  QSize size;
  if (start.length == 5 * file_int_size) {
    pos.setX(reader.readInt());
    pos.setY(reader.readInt());
    size.setWidth(reader.readInt());
    size.setHeight(reader.readInt());
  }
  
  TRY(reader.end());
  
  if (+span.len <= 0) return "Negative cell span length";
  span.cell = std::make_unique<Cell>();
  if (start.length == 5 * file_int_size) {
    // TODO: range checking on pos?
    if (size.width() <= 0 || max_image_width < size.width()) {
      return "Cell width out-of-range";
    }
    if (size.height() <= 0 || max_image_height < size.height()) {
      return "Cell height out-of-range";
    }
    span.cell->pos = pos;
    span.cell->img = {size, qimageFormat(format)};
  }
  
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readCDAT(QIODevice &dev, QImage &image, const Format format) try {
  SCOPE_TIME("readCDAT");

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
  TRY(expectedName(start, chunk_cell_data));
  
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
    
    SCOPE_TIME("inflate");
    
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
      return "Extra image data";
    }
    copyFromByteOrder(image.scanLine(rowIdx), outBuff.data(), outBuffSize, format);
  } else if (rowIdx == image.height()) {
    if (stream.avail_out != outBuffSize) {
      return "Extra image data";
    }
  } else {
    return "Truncated image data";
  }
  
  TRY(reader.end());
  
  return {};
} catch (FileIOError &e) {
  return e.what();
}

Error readAEND(QIODevice &dev) try {
  SCOPE_TIME("readAEND");

  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_anim_end));
  TRY(expectedLength(start, 0));
  TRY(reader.end());
  return {};
} catch (FileIOError &e) {
  return e.what();
}
