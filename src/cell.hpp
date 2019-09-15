//
//  cell.hpp
//  Animera
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef cell_hpp
#define cell_hpp

#include <memory>
#include <vector>
#include "image.hpp"
#include "enum operators.hpp"

class Cell;

// A null pointer to a cell is a null cell
using CellPtr = std::unique_ptr<Cell>;

class Cell {
public:
  Cell(QSize, Format);
  
  QImage image;
};

enum class LayerIdx {};
enum class FrameIdx {};

template <>
struct enum_unary_plus<LayerIdx> : std::true_type {};

template <>
struct enum_unary_plus<FrameIdx> : std::true_type {};

template <>
struct enum_math<LayerIdx> : std::true_type {};

template <>
struct enum_math<FrameIdx> : std::true_type {};

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

struct CellSpan {
  CellPtr cell;
  FrameIdx len{1};
};

using Frame = std::vector<const Cell *>;

#endif
