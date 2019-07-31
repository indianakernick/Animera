//
//  cell span.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 25/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef cell_span_hpp
#define cell_span_hpp

#include "cell.hpp"

class LayerCells {
public:
  class ConstIterator {
    friend LayerCells;
    
  public:
    ConstIterator &operator++();
    const Cell *operator*() const;
  
  private:
    std::vector<CellSpan>::const_iterator iter;
    FrameIdx idx;
  };

  LayerCells() = default;

  CellSpan *data() {
    return spans.data();
  }
  const CellSpan *data() const {
    return spans.data();
  }
  size_t size() const {
    return spans.size();
  }
  auto begin() {
    return spans.begin();
  }
  auto begin() const {
    return spans.begin();
  }
  auto end() {
    return spans.end();
  }
  auto end() const {
    return spans.end();
  }
  void resize(const size_t size) {
    spans.resize(size);
  }
  
  void pushNull(const FrameIdx len) {
    spans.push_back({nullptr, len});
  }
  void pushCell(CellPtr cell) {
    spans.push_back({std::move(cell)});
  }

  /// Get a constant iterator to a cell
  ConstIterator find(FrameIdx) const;
  /// Get a mutable cell
  Cell *get(FrameIdx);
  /// Get a constant cell
  const Cell *get(FrameIdx) const;
  
  /// Insert a copy of the previous cell after the index
  /// If the index refers to the last cell of a span, the span is copied
  /// otherwise the span is extended
  void insertCopy(FrameIdx);

  /// Insert a new cell after the index
  void insertNew(FrameIdx, CellPtr);

  /// Replace a cell with a new cell
  void replaceNew(FrameIdx, CellPtr);

  /// Extend the span at the index and shrink the following span
  /// The span will not be extended past the end
  void extend(FrameIdx);

  /// Split the span at the index into two
  /// The index becomes the first cell of the second span
  void split(FrameIdx);

  /// Replace part of a span with another span
  void replaceSpan(FrameIdx, LayerCells &);

  /// Make a copy of part of a span
  LayerCells extract(FrameIdx, FrameIdx) const;

  /// Make a truncated copy of the spans
  LayerCells truncateCopy(FrameIdx) const;

  /// Remove a cell
  void remove(FrameIdx);

  /// Replace the spans with a single null span
  void clear(FrameIdx);
  
private:
  std::vector<CellSpan> spans;
  
  explicit LayerCells(std::vector<CellSpan>);
};

#endif
