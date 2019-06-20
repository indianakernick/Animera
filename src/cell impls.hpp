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

enum class CellType : uint8_t {
  null,
  source,
  duplicate,
  transform
};

void serializeCell(QIODevice *, const Cell *);
CellPtr deserializeCell(QIODevice *);

class SourceCell final : public Cell {
public:
  SourceCell() = default;
  SourceCell(QSize, Format, Palette *);
  explicit SourceCell(QIODevice *);

  void serialize(QIODevice *) const override;
  Image outputImage() const override;
  void updateInput(const Cell *) override;
  CellPtr clone() const override;
  
  Image image;
};

class DuplicateCell final : public Cell {
public:
  DuplicateCell() = default;
  explicit DuplicateCell(const Cell *);
  explicit DuplicateCell(QIODevice *);

  void serialize(QIODevice *) const override;
  Image outputImage() const override;
  void updateInput(const Cell *) override;
  CellPtr clone() const override;

  const Cell *source = nullptr;
};

class TransformCell final : public Cell {
public:
  TransformCell() = default;
  explicit TransformCell(const Cell *);
  explicit TransformCell(QIODevice *);

  void serialize(QIODevice *) const override;
  Image outputImage() const override;
  void updateInput(const Cell *) override;
  CellPtr clone() const override;
  
  const Cell *source = nullptr;
};

#endif
