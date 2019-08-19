//
//  color.hpp
//  Animera
//
//  Created by Indi Kernick on 25/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef color_hpp
#define color_hpp

#include <cstdint>

struct Color {
  // @TODO is this a bad idea?
  /*
  union {
    uint8_t r;
    uint8_t y;
  };
  */

  uint8_t r, g, b, a;
};

#endif
