//
//  chunk io.hpp
//  Animera
//
//  Created by Indiana Kernick on 18/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_chunk_io_hpp
#define animera_chunk_io_hpp

#include "error.hpp"
#include "config.hpp"
#include "file io error.hpp"

class QIODevice;

class ChunkWriter {
public:
  explicit ChunkWriter(QIODevice &);

  void begin(std::uint32_t, const char *);
  void begin(const char *);
  void end();
  
  void writeByte(std::uint8_t);
  void writeInt(std::uint32_t);
  void writeString(const char *, std::uint32_t);
  void writeString(const signed char *, std::uint32_t);
  void writeString(const unsigned char *, std::uint32_t);

private:
  QIODevice &dev;
  unsigned long crc;
  qint64 startPos;
  
  void writeStart(std::uint32_t, const char *);
  
  template <typename T>
  void writeData(const T *, std::uint32_t);
};

struct ChunkStart {
  std::uint32_t length;
  char name[chunk_name_len];
};

class ChunkReader {
public:
  explicit ChunkReader(QIODevice &);
  
  ChunkStart begin();
  Error end();
  
  ChunkStart peek();
  void skip(ChunkStart);
  
  std::uint8_t readByte();
  std::uint32_t readInt();
  void readString(char *, std::uint32_t);
  void readString(signed char *, std::uint32_t);
  void readString(unsigned char *, std::uint32_t);

private:
  QIODevice &dev;
  unsigned long crc;
  char name[chunk_name_len];
  
  template <typename T>
  void readData(T *, std::uint32_t);
};

Error expectedName(ChunkStart, const char *);
QString chunkLengthInvalid(ChunkStart);

#endif
