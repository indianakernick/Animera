//
//  chunk io.hpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef chunk_io_hpp
#define chunk_io_hpp

#include "error.hpp"
#include "config.hpp"
#include "file io error.hpp"

class QIODevice;

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
  unsigned long crc;
  qint64 startPos;
  
  void writeStart(uint32_t, const char *);
  
  template <typename T>
  void writeData(const T *, uint32_t);
};

struct ChunkStart {
  uint32_t length;
  char name[chunk_name_len];
};

class ChunkReader {
public:
  explicit ChunkReader(QIODevice &);
  
  ChunkStart begin();
  Error end();
  
  uint8_t readByte();
  uint32_t readInt();
  void readString(char *, uint32_t);
  void readString(signed char *, uint32_t);
  void readString(unsigned char *, uint32_t);

private:
  QIODevice &dev;
  unsigned long crc;
  
  template <typename T>
  void readData(T *, uint32_t);
};

Error expectedName(ChunkStart, const char *);
Error expectedLength(ChunkStart, uint32_t);
Error expectedNameLength(ChunkStart, const char *, uint32_t);

#endif
