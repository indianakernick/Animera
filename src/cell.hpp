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
#include "enum operators.hpp"

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
