//
//  animation file.cpp
//  Animera
//
//  Created by Indiana Kernick on 31/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "animation file.hpp"

#include "zlib.hpp"
#include "strings.hpp"
#include "chunk io.hpp"
#include "cel array.hpp"
#include "scope time.hpp"
#include <Graphics/format.hpp>

namespace {

Bytef *getImageRowBuffer(const std::size_t newSize) {
  static std::unique_ptr<Bytef[]> buffer;
  static std::size_t size = 0;
  if (size < newSize) {
    // TODO: std::make_unique_for_overwrite
    buffer = std::unique_ptr<Bytef[]>{new Bytef[newSize]};
    size = newSize;
  }
  return buffer.get();
}

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

constexpr std::uint8_t formatByte(const Format format) {
  return byteDepth(format);
}

template <typename DstFmt, typename SrcFmt>
void alignedCopy(unsigned char *dstBytes, const unsigned char *srcBytes, const std::size_t size) {
  using DstPixel = typename DstFmt::Pixel;
  using SrcPixel = typename SrcFmt::Pixel;
  
  static_assert(sizeof(DstPixel) == sizeof(SrcPixel));
  assert(size % sizeof(DstPixel) == 0);
  assert(reinterpret_cast<std::uintptr_t>(dstBytes) % sizeof(DstPixel) == 0);
  assert(reinterpret_cast<std::uintptr_t>(srcBytes) % sizeof(SrcPixel) == 0);
  
  if constexpr (std::is_same_v<DstFmt, SrcFmt>) {
    std::memcpy(dstBytes, srcBytes, size);
  } else {
    auto *dst = reinterpret_cast<DstPixel *>(dstBytes);
    const auto *src = reinterpret_cast<const SrcPixel *>(srcBytes);
    auto *dstEnd = reinterpret_cast<DstPixel *>(dstBytes + size);
    
    while (dst != dstEnd) {
      *dst++ = DstFmt::pixel(SrcFmt::color(*src++));
    }
  }
}

void copyToByteOrder(
  unsigned char *dst,
  const unsigned char *src,
  const std::size_t size,
  const Format format
) {
  SCOPE_TIME("copyToByteOrder");

  assert(size % byteDepth(format) == 0);
  switch (format) {
    case Format::rgba:
      alignedCopy<RGBA, FmtRgba>(dst, src, size);
      break;
    case Format::index:
      std::memcpy(dst, src, size);
      break;
    case Format::gray: {
      alignedCopy<YA, FmtGray>(dst, src, size);
      break;
    }
  }
}

void copyFromByteOrder(
  unsigned char *dst,
  const unsigned char *src,
  const std::size_t size,
  const Format format
) {
  SCOPE_TIME("copyFromByteOrder");

  assert(size % byteDepth(format) == 0);
  switch (format) {
    case Format::rgba:
      alignedCopy<FmtRgba, RGBA>(dst, src, size);
      break;
    case Format::index:
      std::memcpy(dst, src, size);
      break;
    case Format::gray:
      alignedCopy<FmtGray, YA>(dst, src, size);
      break;
  }
}

}

Error writeSignature(QIODevice &dev) {
  SCOPE_TIME("writeSignature");

  if (dev.write(file_sig, file_sig_len) != file_sig_len) {
    return dev.errorString();
  }
  return {};
}

Error writeAHDR(QIODevice &dev, const AnimationInfo &info) try {
  SCOPE_TIME("writeAHDR");

  ChunkWriter writer{dev};
  writer.begin(6 * file_int_size + 1, chunk_anim_header);
  writer.writeInt(info.width);
  writer.writeInt(info.height);
  writer.writeInt(static_cast<std::uint32_t>(info.layers));
  writer.writeInt(static_cast<std::uint32_t>(info.groups));
  writer.writeInt(static_cast<std::uint32_t>(info.frames));
  writer.writeInt(info.delay);
  writer.writeByte(formatByte(info.format));
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

namespace {

Error writeRgba(QIODevice &dev, const PaletteCSpan colors) try {
  ChunkWriter writer{dev};
  writer.begin(static_cast<std::uint32_t>(colors.size()) * 4, chunk_palette);
  for (std::size_t i = 0; i != colors.size(); ++i) {
    const gfx::Color color = FmtRgba::color(static_cast<PixelRgba>(colors[i]));
    writer.writeByte(color.r);
    writer.writeByte(color.g);
    writer.writeByte(color.b);
    writer.writeByte(color.a);
  }
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

Error writeGray(QIODevice &dev, const PaletteCSpan colors) try {
  ChunkWriter writer{dev};
  writer.begin(static_cast<std::uint32_t>(colors.size()) * 2, chunk_palette);
  for (std::size_t i = 0; i != colors.size(); ++i) {
    const gfx::Color color = FmtGray::color(static_cast<PixelGray>(colors[i]));
    writer.writeByte(color.r);
    writer.writeByte(color.a);
  };
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.msg();
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
  return e.msg();
}

Error writeGRPS(QIODevice &dev, const std::vector<Group> &groups) try {
  SCOPE_TIME("writeGRPS");
  
  ChunkWriter writer{dev};
  writer.begin(chunk_groups);
  FrameIdx prevEnd{};
  
  for (const Group &group : groups) {
    const auto groupLen = static_cast<std::uint32_t>(group.end - prevEnd);
    const auto nameLen = static_cast<std::uint32_t>(group.name.size());
    prevEnd = group.end;
    writer.writeInt(groupLen);
    writer.writeInt(nameLen);
    writer.writeString(group.name.data(), nameLen);
  }
  
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

Error writeLHDR(QIODevice &dev, const Layer &layer) try {
  SCOPE_TIME("writeLHDR");

  ChunkWriter writer{dev};
  const std::uint32_t nameLen = static_cast<std::uint32_t>(layer.name.size());
  writer.begin(file_int_size + 1 + nameLen, chunk_layer_header);
  writer.writeInt(static_cast<std::uint32_t>(layer.cels.size()));
  writer.writeByte(layer.visible);
  writer.writeString(layer.name.data(), nameLen);
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

Error writeCHDR(QIODevice &dev, const Cel &cel) try {
  SCOPE_TIME("writeCHDR");

  ChunkWriter writer{dev};
  writer.begin(chunk_cel_header);
  writer.writeInt(static_cast<std::uint32_t>(cel.len));
  if (*cel.cel) {
    const QRect rect = cel.cel->rect();
    writer.writeInt(rect.x());
    writer.writeInt(rect.y());
    writer.writeInt(rect.width());
    writer.writeInt(rect.height());
  }
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

namespace {

struct CompressContext {
  ChunkWriter &writer;
  const QImage &image;
  Bytef *inBuff;
  uInt inBuffSize;
  Format format;
  std::uint32_t remainingChunk = ~std::uint32_t{};
  int rowIdx = 0;
  
  CompressContext(ChunkWriter &writer, const QImage &image, const Format format)
    : writer{writer}, image{image}, format{format} {
    inBuffSize = image.width() * byteDepth(format);
    inBuff = getImageRowBuffer(inBuffSize);
  }
  
  bool hasInput() const {
    return rowIdx < image.height();
  }
  
  std::pair<const Bytef *, uInt> getInputBuffer() {
    copyToByteOrder(inBuff, image.scanLine(rowIdx++), inBuffSize, format);
    return {inBuff, inBuffSize};
  }
  
  Error processOutputBuffer(const Bytef *dat, const uInt len) {
    // This situation is near impossible. This might as well be an assert
    if (remainingChunk < len) return "Chunk overflow";
    writer.writeString(dat, len);
    remainingChunk -= len;
    return {};
  }
};

}

Error writeCDAT(QIODevice &dev, const QImage &image, const Format format) try {
  SCOPE_TIME("writeCDAT");
  
  static_assert(sizeof(uInt) >= sizeof(std::uint32_t));
  
  assert(!image.isNull());
  
  ChunkWriter writer{dev};
  writer.begin(chunk_cel_data);
  CompressContext context{writer, image, format};
  TRY(zlibCompress(context));
  writer.end();
  
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

Error writeAEND(QIODevice &dev) try {
  SCOPE_TIME("writeAEND");

  ChunkWriter writer{dev};
  writer.begin(0, chunk_anim_end);
  writer.end();
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

Error readSignature(QIODevice &dev) {
  SCOPE_TIME("readSignature");

  char signature[file_sig_len];
  if (dev.read(signature, file_sig_len) != file_sig_len) {
    return dev.errorString();
  }
  if (std::memcmp(signature, file_sig, file_sig_len) != 0) {
    return "File signature mismatch";
  }
  return {};
}

namespace {

Error readFormatByte(Format &format, const std::uint8_t byte) {
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

Error readAHDR(QIODevice &dev, AnimationInfo &info) try {
  SCOPE_TIME("readAHDR");

  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_anim_header));
  if (start.length != 6 * file_int_size + 1) {
    return chunkLengthInvalid(start);
  }
  
  info.width = reader.readInt();
  info.height = reader.readInt();
  info.layers = static_cast<LayerIdx>(reader.readInt());
  info.groups = static_cast<GroupIdx>(reader.readInt());
  info.frames = static_cast<FrameIdx>(reader.readInt());
  info.delay = reader.readInt();
  const std::uint8_t format = reader.readByte();
  
  TRY(reader.end());
  
  if (info.width < init_size_range.min || init_size_range.max < info.width) {
    return "Canvas width is out-of-range";
  }
  if (info.height < init_size_range.min || init_size_range.max < info.height) {
    return "Canvas height is out-of-range";
  }
  if (+info.layers <= 0) return "Layer count is out-of-range";
  if (+info.groups <= 0) return "Group count is out-of-range";
  if (+info.frames <= 0) return "Frame count is out-of-range";
  if (info.delay < ctrl_delay.min || ctrl_delay.max < info.delay) {
    return "Animation delay is out-of-range";
  }
  TRY(readFormatByte(info.format, format));
  
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

namespace {

Error checkPaletteStart(ChunkStart start, const int multiple) {
  TRY(expectedName(start, chunk_palette));
  if (start.length % multiple != 0 || start.length / multiple > pal_colors) {
    return chunkLengthInvalid(start);
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
    *iter = PixelVar{FmtRgba::pixel(color)};
  }
  TRY(reader.end());
  std::fill(iter, colors.end(), PixelVar{});
  return {};
} catch (FileIOError &e) {
  return e.msg();
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
    *iter = PixelVar{FmtGray::pixel(color)};
  }
  TRY(reader.end());
  std::fill(iter, colors.end(), PixelVar{});
  return {};
} catch (FileIOError &e) {
  return e.msg();
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
  return e.msg();
}

namespace {

bool validName(const std::string_view name) {
  for (const char ch : name) {
    if (!printable(ch)) return false;
  }
  return true;
}

}

Error readGRPS(QIODevice &dev, std::vector<Group> &groups, const FrameIdx frameCount) try {
  SCOPE_TIME("readGRPS");
  
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_groups));
  std::size_t remaining = start.length;
  FrameIdx totalLen{};
  std::size_t index = 0;
  
  while (remaining > 2 * file_int_size) {
    if (index >= groups.size()) {
      return "Number of groups found does not equal the group count";
    }
    
    const auto groupLen = static_cast<FrameIdx>(reader.readInt());
    const std::uint32_t nameLen = reader.readInt();
    remaining -= 2 * file_int_size;
    
    if (groupLen <= FrameIdx{0}) {
      return "Group length is negative";
    }
    totalLen += groupLen;
    
    if (nameLen > layer_name_max_len || nameLen > remaining) {
      return "Group name exceeds maximum length";
    }
    remaining -= nameLen;
    
    groups[index].end = totalLen;
    groups[index].name.resize(nameLen);
    reader.readString(groups[index].name.data(), nameLen);
    
    if (!validName(groups[index].name)) {
      return "Group name contains non-ASCII characters";
    }
    ++index;
  }
  
  TRY(reader.end());
  
  if (remaining != 0) {
    return chunkLengthInvalid(start);
  }
  if (index < groups.size()) {
    return "Number of groups found does not equal the group count";
  }
  if (groups.back().end != frameCount) {
    return "Total length of groups does not equal the frame count";
  }
  
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

namespace {

Error readVisibleByte(bool &visible, const std::uint8_t byte) {
  switch (byte) {
    case 0:
      visible = false;
      break;
    case 1:
      visible = true;
      break;
    default:
      return "Invalid visibility (" + QString::number(byte) + ")";
  }
  return {};
}

}

Error readLHDR(QIODevice &dev, Layer &layer) try {
  SCOPE_TIME("readLHDR");

  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_layer_header));
  const std::uint32_t nameLen = start.length - (file_int_size + 1);
  if (start.length <= file_int_size + 1 || nameLen > layer_name_max_len) {
    return chunkLengthInvalid(start);
  }
  
  const std::uint32_t cels = reader.readInt();
  const std::uint8_t visible = reader.readByte();
  
  layer.name.resize(nameLen);
  reader.readString(layer.name.data(), nameLen);
  
  TRY(reader.end());
  
  if (cels == 0) return "Layer cels out-of-range";
  if (!validName(layer.name)) {
    return "Layer name contains non-ASCII characters";
  }
  TRY(readVisibleByte(layer.visible, visible));
  
  layer.cels.resize(cels);
  
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

Error readCHDR(QIODevice &dev, Cel &cel, const Format format) try {
  SCOPE_TIME("readCHDR");

  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_cel_header));
  if (start.length != file_int_size && start.length != 5 * file_int_size) {
    return chunkLengthInvalid(start);
  }
  
  cel.len = static_cast<FrameIdx>(reader.readInt());
  QPoint pos;
  QSize size;
  if (start.length == 5 * file_int_size) {
    pos.setX(reader.readInt());
    pos.setY(reader.readInt());
    size.setWidth(reader.readInt());
    size.setHeight(reader.readInt());
  }
  
  TRY(reader.end());
  
  if (+cel.len <= 0) return "Negative cel length";
  cel.cel = std::make_unique<CelImage>();
  if (start.length == 5 * file_int_size) {
    if (size.width() <= 0 || max_image_width < size.width()) {
      return "Cel width out-of-range";
    }
    if (size.height() <= 0 || max_image_height < size.height()) {
      return "Cel height out-of-range";
    }
    cel.cel->pos = pos;
    cel.cel->img = {size, qimageFormat(format)};
  }
  
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

namespace {

struct DecompressContext {
  ChunkReader &reader;
  QImage &image;
  std::uint32_t remainingChunk;
  Bytef *outBuff;
  uInt outBuffSize;
  int rowIdx = 0;
  Format format;
  
  DecompressContext(
    ChunkReader &reader,
    QImage &image,
    const Format format,
    const std::uint32_t remainingChunk
  ) : reader{reader},
      image{image},
      remainingChunk{remainingChunk},
      format{format} {
    outBuffSize = image.width() * byteDepth(format);
    outBuff = getImageRowBuffer(outBuffSize);
  }
  
  bool hasInput() const {
    return remainingChunk != 0;
  }
  
  bool hasOutput() {
    return rowIdx < image.height();
  }
  
  bool hasLastOutput() {
    return rowIdx == image.height() - 1;
  }
  
  uInt fillInputBuffer(Bytef *dat, uInt len) {
    len = std::min(len, remainingChunk);
    remainingChunk -= len;
    reader.readString(dat, len);
    return len;
  }

  std::pair<Bytef *, uInt> getOutputBuffer() {
    return {outBuff, outBuffSize};
  }
  
  void processOutputBuffer() {
    copyFromByteOrder(image.scanLine(rowIdx++), outBuff, outBuffSize, format);
  }
};

}

Error readCDAT(QIODevice &dev, QImage &image, const Format format) try {
  SCOPE_TIME("readCDAT");

  assert(!image.isNull());
  
  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_cel_data));
  DecompressContext context{reader, image, format, start.length};
  TRY(zlibDecompress(context));
  return reader.end();
} catch (FileIOError &e) {
  return e.msg();
}

Error readAEND(QIODevice &dev) try {
  SCOPE_TIME("readAEND");

  ChunkReader reader{dev};
  const ChunkStart start = reader.begin();
  TRY(expectedName(start, chunk_anim_end));
  if (start.length != 0) {
    return chunkLengthInvalid(start);
  }
  return reader.end();
} catch (FileIOError &e) {
  return e.msg();
}
