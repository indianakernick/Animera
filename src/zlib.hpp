//
//  zlib.hpp
//  Animera
//
//  Created by Indi Kernick on 7/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef zlib_hpp
#define zlib_hpp

#define ZLIB_CONST

#include <zlib.h>
#include "error.hpp"

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

#endif
