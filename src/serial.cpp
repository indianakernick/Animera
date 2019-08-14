//
//  serial.cpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "serial.hpp"

#include <QtCore/qendian.h>

ChunkWriter::ChunkWriter(QIODevice &dev)
  : dev{dev} {}

void ChunkWriter::begin(const uint32_t len, const char *header) {
  startPos = -1;
  writeHeader(len, header);
}

void ChunkWriter::begin(const char *header) {
  assert(!dev.isSequential());
  startPos = dev.pos();
  writeHeader(0, header);
}

void ChunkWriter::end() {
  if (startPos != -1) {
    const qint64 currPos = dev.pos();
    const uint32_t dataLen = static_cast<uint32_t>(currPos - startPos - 8);
    dev.seek(startPos);
    writeInt(dataLen);
    dev.seek(currPos);
  }
  writeInt(static_cast<uint32_t>(crc));
}

void ChunkWriter::writeByte(const uint8_t byte) {
  writeData(&byte);
}

void ChunkWriter::writeInt(uint32_t num) {
  num = qToBigEndian(num);
  writeData(&num);
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
  dev.write(reinterpret_cast<const char *>(dat), len);
  crc = crc32(crc, reinterpret_cast<const Bytef *>(dat), len);
}
