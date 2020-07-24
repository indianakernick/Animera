//
//  zlib.hpp
//  Animera
//
//  Created by Indiana Kernick on 7/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_zlib_hpp
#define animera_zlib_hpp

#define ZLIB_CONST

#include <zlib.h>
#include <memory>
#include "error.hpp"
#include "scope time.hpp"
#include "config geometry.hpp"

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

inline Bytef *getZlibBuffer() {
  // TODO: std::make_unique_for_overwrite
  static auto buffer = std::unique_ptr<Bytef[]>{new Bytef[file_buff_size]};
  return buffer.get();
}

/*
struct Context {
  bool hasInput();
  std::pair<const Bytef *, uInt> getInputBuffer();
  Error or void processOutputBuffer(const Bytef *, uInt);
};
*/

template <typename Context>
Error zlibCompress(Context ctx) {
  const uInt outBuffSize = file_buff_size;
  Bytef *outBuff = getZlibBuffer();
  
  z_stream stream;
  stream.zalloc = nullptr;
  stream.zfree = nullptr;
  int ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
  if (ret == Z_MEM_ERROR) return "zlib: memory error";
  assert(ret == Z_OK);
  const std::unique_ptr<z_stream, DeflateDeleter> deleter{&stream};
  
  stream.avail_in = 0;
  stream.next_out = outBuff;
  stream.avail_out = outBuffSize;
  
  do {
    if (stream.avail_in == 0 && ctx.hasInput()) {
      const std::pair<const Bytef *, uInt> input = ctx.getInputBuffer();
      stream.next_in = input.first;
      stream.avail_in = input.second;
    }
    
    if (stream.avail_out == 0) {
      const Bytef *constOutBuff = outBuff;
      TRY_VOID(ctx.processOutputBuffer(constOutBuff, outBuffSize));
      stream.next_out = outBuff;
      stream.avail_out = outBuffSize;
    }
    
    SCOPE_TIME("deflate");
    
    ret = deflate(&stream, stream.avail_in ? Z_NO_FLUSH : Z_FINISH);
  } while (ret == Z_OK);
  assert(ret == Z_STREAM_END);
  
  if (stream.avail_out < outBuffSize) {
    const Bytef *constOutBuff = outBuff;
    TRY_VOID(ctx.processOutputBuffer(constOutBuff, outBuffSize - stream.avail_out));
  }
  
  return {};
}

/*
struct Context {
  bool hasInput();
  bool hasOutput();
  bool hasLastOutput();
  uInt fillInputBuffer(Bytef *, uInt);
  std::pair<Bytef *, uInt> getOutputBuffer();
  Error or void processOutputBuffer();
};
*/

template <typename Context>
Error zlibDecompress(Context ctx) {
  const uInt inBuffSize = file_buff_size;
  Bytef *inBuff = getZlibBuffer();
  
  z_stream stream;
  stream.zalloc = nullptr;
  stream.zfree = nullptr;
  int ret = inflateInit(&stream);
  if (ret == Z_MEM_ERROR) return "zlib: memory error";
  assert(ret == Z_OK);
  const std::unique_ptr<z_stream, InflateDeleter> deleter{&stream};
  
  uInt outBuffSize;
  stream.avail_in = 0;
  {
    const std::pair<Bytef *, uInt> output = ctx.getOutputBuffer();
    stream.next_out = output.first;
    stream.avail_out = output.second;
    outBuffSize = output.second;
  }
  
  do {
    if (stream.avail_in == 0 && ctx.hasInput()) {
      const uInt input = ctx.fillInputBuffer(inBuff, inBuffSize);
      stream.next_in = inBuff;
      stream.avail_in = input;
    }
    
    if (stream.avail_out == 0 && ctx.hasOutput()) {
      TRY_VOID(ctx.processOutputBuffer());
      const std::pair<Bytef *, uInt> output = ctx.getOutputBuffer();
      stream.next_out = output.first;
      stream.avail_out = output.second;
      outBuffSize = output.second;
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
  
  if (ctx.hasLastOutput()) {
    if (stream.avail_out != 0) {
      return "zlib: extra data";
    }
    ctx.processOutputBuffer();
  } else if (!ctx.hasOutput()) {
    if (stream.avail_out != outBuffSize) {
      return "zlib: extra data";
    }
  } else {
    return "zlib: truncated data";
  }
  
  return {};
}

#endif
