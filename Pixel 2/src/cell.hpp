//
//  cell.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef pixel_2_cell_hpp
#define pixel_2_cell_hpp

#include "image.hpp"

class Cell {
public:
  virtual ~Cell();
  virtual Image image() const = 0;
};

#endif
