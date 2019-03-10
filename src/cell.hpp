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
#include <vector>
#include "image.hpp"

class Cell;

using CellPtr = std::unique_ptr<Cell>;

class Cell {
public:
  virtual ~Cell();
  
  virtual void serialize(QIODevice *) const = 0;
  virtual Image outputImage() const = 0;
  virtual void updateInput(const Cell *) = 0;
  virtual CellPtr clone() const = 0;
};

using LayerIdx = uint32_t;
using FrameIdx = uint32_t;

struct CellPos {
  LayerIdx l;
  FrameIdx f;
};

struct CellRect {
  LayerIdx minL;
  LayerIdx maxL;
  FrameIdx minF;
  FrameIdx maxF;
};

using Frames = std::vector<CellPtr>;
using Layers = std::vector<Frames>;
using Frame = std::vector<Cell *>;
using LayerVisible = std::vector<bool>;

#endif
