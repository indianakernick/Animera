//
//  cell.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef cell_hpp
#define cell_hpp

#include <memory>
#include "image.hpp"

class Cell;

using CellPtr = std::unique_ptr<Cell>;

class Cell {
public:
  virtual ~Cell();
  
  virtual Image outputImage() const = 0;
  virtual void updateInput(const Cell *) = 0;
  virtual CellPtr clone() const = 0;
  virtual void serialize(QIODevice *) const = 0;
};

#endif
