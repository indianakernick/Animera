//
//  serial.cpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "serial.hpp"

#include <QtCore/qendian.h>

const char *FileIOError::what() const noexcept {
  return "File IO error";
}

ChunkWriter::ChunkWriter(QIODevice &dev)
  : dev{dev} {}

void ChunkWriter::begin(const uint32_t len, const char *header) {
  startPos = -1;
  writeHeader(len, header);
}

void ChunkWriter::begin(const char *header) {
  assert(!dev.isSequential());
  startPos = dev.pos();
  assert(startPos != 0);
  writeHeader(0, header);
}

void ChunkWriter::end() {
  if (startPos != -1) {
    const qint64 currPos = dev.pos();
    assert(currPos != 0);
    const uint32_t dataLen = static_cast<uint32_t>(currPos - startPos - 8);
    assert(qint64{dataLen} == currPos - startPos - 8);
    if (!dev.seek(startPos)) throw FileIOError{};
    writeInt(dataLen);
    if (!dev.seek(currPos)) throw FileIOError{};
  }
  writeInt(static_cast<uint32_t>(crc));
}

void ChunkWriter::writeByte(const uint8_t byte) {
  writeData(&byte, 1);
}

void ChunkWriter::writeInt(uint32_t num) {
  num = qToBigEndian(num);
  writeData(&num, 4);
}

void ChunkWriter::writeString(const char *dat, const uint32_t len) {
  writeData(dat, len);
}

void ChunkWriter::writeString(const signed char *dat, const uint32_t len) {
  writeData(dat, len);
}

void ChunkWriter::writeString(const unsigned char *dat, const uint32_t len) {
  writeData(dat, len);
}

void ChunkWriter::writeHeader(const uint32_t len, const char *header) {
  writeInt(len);
  crc = crc32(0, nullptr, 0);
  writeString(header, 4);
}

template <typename T>
void ChunkWriter::writeData(const T *dat, const uint32_t len) {
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
  readString(start.header, 4);
  return start;
}

Error ChunkReader::end() {
  const uint32_t finalCrc = static_cast<uint32_t>(crc);
  if (finalCrc != readInt()) {
    return "CRC mismatch";
  } else {
    return {};
  }
}

uint8_t ChunkReader::readByte() {
  uint8_t byte;
  readData(&byte, 1);
  return byte;
}

uint32_t ChunkReader::readInt() {
  uint32_t num;
  readData(&num, 4);
  return qFromBigEndian(num);
}

void ChunkReader::readString(char *dat, const uint32_t len) {
  readData(dat, len);
}

void ChunkReader::readString(signed char *dat, const uint32_t len) {
  readData(dat, len);
}

void ChunkReader::readString(unsigned char *dat, const uint32_t len) {
  readData(dat, len);
}

template <typename T>
void ChunkReader::readData(T *dat, const uint32_t len) {
  if (dev.read(reinterpret_cast<char *>(dat), len) != len) {
    throw FileIOError{};
  }
  crc = crc32(crc, reinterpret_cast<const Bytef *>(dat), len);
}

Error expectedHeader(const ChunkStart start, const char *header) {
  if (std::memcmp(start.header, header, 4) != 0) {
    QString msg = "Expected '";
    msg += QLatin1String{header, 4};
    msg += "' chunk but found '";
    msg += QLatin1String{start.header, 4};
    msg += '\'';
    return msg;
  } else {
    return {};
  }
}

Error expectedLength(const ChunkStart start, const uint32_t length) {
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
