//
//  cell.hpp
//  Animera
//
//  Created by Indiana Kernick on 17/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef cell_hpp
#define cell_hpp

#include <memory>
#include <vector>
#include "image.hpp"
#include "enum operators.hpp"

class Cell;

// we need stable cell pointers
using CellPtr = std::unique_ptr<Cell>;

class Cell {
public:
  QImage img;
  QPoint pos;
  
  explicit operator bool() const {
    return !img.isNull();
  }
  bool isNull() const {
    return img.isNull();
  }
  QRect rect() const {
    return {pos, img.size()};
  }
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

constexpr CellRect empty_rect = {LayerIdx{0}, FrameIdx{0}, LayerIdx{-1}, FrameIdx{-1}};

struct CellSpan {
  CellPtr cell;
  FrameIdx len{1};
};

using Frame = std::vector<const Cell *>;

#endif
