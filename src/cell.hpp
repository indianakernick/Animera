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

// @TODO does this class really need to exist anymore? It's just a QImage
class Cell {
public:
  Cell() = default;
  explicit Cell(QImage);
  Cell(QSize, Format);
  
  CellPtr clone() const;
  
  QImage image;
};

using LayerIdx = int;
using FrameIdx = int;

struct CellPos {
  LayerIdx l;
  FrameIdx f;
};

struct CellRect {
  LayerIdx minL;
  FrameIdx minF;
  LayerIdx maxL;
  FrameIdx maxF;
};

// @TODO is CellSpan too similar to std::span?
struct CellSpan {
  CellPtr cell;
  FrameIdx len = 1;
};

using Spans = std::vector<CellSpan>;

struct Layer {
  Spans spans;
  std::string name;
  bool visible = true;
};

using Layers = std::vector<Layer>;
using Frame = std::vector<const Cell *>;

#endif
