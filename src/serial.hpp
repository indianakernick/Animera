//
//  serial.hpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef serial_hpp
#define serial_hpp

#include "zlib.hpp"
#include <QtCore/qendian.h>
#include <QtCore/qiodevice.h>

template <typename T>
void serializeBytes(QIODevice *dev, const T &data) {
  assert(dev);
  assert(dev->isWritable());
  dev->write(reinterpret_cast<const char *>(&data), sizeof(T));
}

template <typename T>
void deserializeBytes(QIODevice *dev, T &data) {
  assert(dev);
  assert(dev->isReadable());
  dev->read(reinterpret_cast<char *>(&data), sizeof(T));
}

template <typename T>
T deserializeBytesAs(QIODevice *dev) {
  T data;
  deserializeBytes(dev, data);
  return data;
}

class ChunkWriter {
public:
  explicit ChunkWriter(QIODevice &dev)
    : dev{dev} {}

  void begin(const uint32_t len, const char *header) {
    startPos = -1;
    writeHeader(len, header);
  }
  
  void begin(const char *header) {
    assert(!dev.isSequential());
    startPos = dev.pos();
    writeHeader(0, header);
  }
  
  void writeByte(const uint8_t byte) {
    writeToDev(&byte);
    updateCRC(&byte);
  }
  
  void writeInt(uint32_t num) {
    num = qToBigEndian(num);
    writeToDev(&num);
    updateCRC(&num);
  }
  
  template <typename Char>
  std::enable_if_t<sizeof(Char) == 1 && std::is_integral_v<Char>>
  writeString(const Char *dat, const uint32_t len) {
    writeToDev(dat, len);
    updateCRC(dat, len);
  }
  
  void end() {
    if (startPos != -1) {
      const qint64 currPos = dev.pos();
      const uint32_t dataLen = static_cast<uint32_t>(currPos - (startPos + 4));
      dev.seek(startPos);
      writeInt(dataLen);
      dev.seek(currPos);
    }
    writeInt(static_cast<uint32_t>(crc));
  }

private:
  QIODevice &dev;
  uLong crc;
  qint64 startPos;
  
  void writeHeader(const uint32_t len, const char *header) {
    writeInt(len);
    crc = crc32(0, nullptr, 0);
    writeString(header, 4);
  }
  
  template <typename T>
  void writeToDev(const T *dat, const uint32_t len = sizeof(T)) {
    dev.write(reinterpret_cast<const char *>(dat), len);
  }
  
  template <typename T>
  void updateCRC(const T *dat, const uint32_t len = sizeof(T)) {
    crc = crc32(crc, reinterpret_cast<const Bytef *>(dat), len);
  }
};

#endif
