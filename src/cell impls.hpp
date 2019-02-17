//
//  cell impls.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef cell_impls_hpp
#define cell_impls_hpp

#include "cell.hpp"

class SourceCell : public Cell {
public:
  SourceCell() = default;
  SourceCell(QSize, Format);

  Image outputImage() const override;
  void updateInput(const Cell *) override;
  CellPtr clone() const override;
  
  Image image;
};

class DuplicateCell : public Cell {
public:
  DuplicateCell() = default;
  explicit DuplicateCell(const Cell *);

  Image outputImage() const override;
  void updateInput(const Cell *) override;
  CellPtr clone() const override;

  const Cell *source = nullptr;
};

class TransformCell : public Cell {
public:
  TransformCell() = default;
  explicit TransformCell(const Cell *);

  Image outputImage() const override;
  void updateInput(const Cell *) override;
  CellPtr clone() const override;
  
  const Cell *source = nullptr;
  Transform xform;
};

#endif
