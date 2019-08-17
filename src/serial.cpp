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
