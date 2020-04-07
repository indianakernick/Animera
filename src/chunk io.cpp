//
//  serial.cpp
//  Animera
//
//  Created by Indiana Kernick on 18/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "chunk io.hpp"

#include "zlib.hpp"
#include <QtCore/qendian.h>
#include <QtCore/qiodevice.h>

ChunkWriter::ChunkWriter(QIODevice &dev)
  : dev{dev} {}

void ChunkWriter::begin(const std::uint32_t len, const char *name) {
  startPos = -1;
  writeStart(len, name);
}

void ChunkWriter::begin(const char *name) {
  assert(!dev.isSequential());
  startPos = dev.pos();
  assert(startPos != 0);
  writeStart(0, name);
}

void ChunkWriter::end() {
  const std::uint32_t finalCrc = static_cast<std::uint32_t>(crc);
  if (startPos != -1) {
    const qint64 currPos = dev.pos();
    assert(currPos != 0);
    const qint64 dataLen = currPos - startPos - chunk_name_len - file_int_size;
    assert(dataLen == qint64{static_cast<std::uint32_t>(dataLen)});
    if (!dev.seek(startPos)) throw FileIOError{};
    writeInt(static_cast<std::uint32_t>(dataLen));
    if (!dev.seek(currPos)) throw FileIOError{};
  }
  writeInt(finalCrc);
}

void ChunkWriter::writeByte(const std::uint8_t byte) {
  writeData(&byte, 1);
}

void ChunkWriter::writeInt(std::uint32_t num) {
  static_assert(sizeof(std::uint32_t) == file_int_size);
  num = qToBigEndian(num);
  writeData(&num, file_int_size);
}

void ChunkWriter::writeString(const char *dat, const std::uint32_t len) {
  writeData(dat, len);
}

void ChunkWriter::writeString(const signed char *dat, const std::uint32_t len) {
  writeData(dat, len);
}

void ChunkWriter::writeString(const unsigned char *dat, const std::uint32_t len) {
  writeData(dat, len);
}

void ChunkWriter::writeStart(const std::uint32_t len, const char *name) {
  writeInt(len);
  crc = crc32(0, nullptr, 0);
  writeString(name, chunk_name_len);
}

template <typename T>
void ChunkWriter::writeData(const T *dat, const std::uint32_t len) {
  if (dev.write(reinterpret_cast<const char *>(dat), len) != len) {
    throw FileIOError{};
  }
  crc = crc32(crc, reinterpret_cast<const Bytef *>(dat), len);
}

ChunkReader::ChunkReader(QIODevice &dev)
  : dev{dev} {}

ChunkStart ChunkReader::begin() {
  ChunkStart start;
  start.length = readInt();
  crc = crc32(0, nullptr, 0);
  readString(start.name, chunk_name_len);
  std::memcpy(name, start.name, chunk_name_len);
  return start;
}

Error ChunkReader::end() {
  const std::uint32_t finalCrc = static_cast<std::uint32_t>(crc);
  if (finalCrc != readInt()) {
    QString msg = "CRC mismatch in '";
    msg += QLatin1String{name, chunk_name_len};
    msg += "' chunk";
    return msg;
  } else {
    return {};
  }
}

std::uint8_t ChunkReader::readByte() {
  std::uint8_t byte;
  readData(&byte, 1);
  return byte;
}

std::uint32_t ChunkReader::readInt() {
  static_assert(sizeof(std::uint32_t) == file_int_size);
  std::uint32_t num;
  readData(&num, file_int_size);
  return qFromBigEndian(num);
}

void ChunkReader::readString(char *dat, const std::uint32_t len) {
  readData(dat, len);
}

void ChunkReader::readString(signed char *dat, const std::uint32_t len) {
  readData(dat, len);
}

void ChunkReader::readString(unsigned char *dat, const std::uint32_t len) {
  readData(dat, len);
}

template <typename T>
void ChunkReader::readData(T *dat, const std::uint32_t len) {
  if (dev.read(reinterpret_cast<char *>(dat), len) != len) {
    throw FileIOError{};
  }
  crc = crc32(crc, reinterpret_cast<const Bytef *>(dat), len);
}

Error expectedName(const ChunkStart start, const char *name) {
  if (std::memcmp(start.name, name, chunk_name_len) != 0) {
    QString msg = "Expected '";
    msg += name;
    msg += "' chunk but found '";
    msg += QLatin1String(start.name, chunk_name_len);
    msg += '\'';
    return msg;
  } else {
    return {};
  }
}

Error expectedLength(const ChunkStart start, const std::uint32_t length) {
  if (start.length != length) {
    QString msg = "Expected chunk size ";
    msg += QString::number(length);
    msg += " but found ";
    msg += QString::number(start.length);
    return msg;
  } else {
    return {};
  }
}
