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
#include <QtCore/qiodevice.h>

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

class FileIOError final : public std::exception {
public:
  const char *what() const noexcept override;
};

class ChunkWriter {
public:
  explicit ChunkWriter(QIODevice &);

  void begin(uint32_t, const char *);
  void begin(const char *);
  void end();
  
  void writeByte(uint8_t);
  void writeInt(uint32_t);
  void writeString(const char *, uint32_t);
  void writeString(const signed char *, uint32_t);
  void writeString(const unsigned char *, uint32_t);

private:
  QIODevice &dev;
  uLong crc;
  qint64 startPos;
  
  void writeHeader(uint32_t, const char *);
  
  template <typename T>
  void writeData(const T *, uint32_t = sizeof(T));
};

#endif
