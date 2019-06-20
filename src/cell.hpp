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
  Cell() = default;
  Cell(QSize, Format, Palette *);
  
  CellPtr clone() const;
  
  Image image;
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
