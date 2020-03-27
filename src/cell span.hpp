//
//  cell span.hpp
//  Animera
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
  std::size_t size() const {
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
  void resize(const std::size_t size) {
    spans.resize(size);
  }
  
  void pushNull(const FrameIdx len) {
    spans.push_back({std::make_unique<Cell>(), len});
  }

  /// Combine consecutive null spans into a single span
  void optimize();
  
  /// Get a constant iterator to a cell
  ConstIterator find(FrameIdx) const;
  /// Get a mutable cell
  Cell *get(FrameIdx);
  /// Get a constant cell
  const Cell *get(FrameIdx) const;
  
  /// Insert a new cell after the index
  void insert(FrameIdx);
  /// Replace a cell with a new cell
  void replace(FrameIdx, bool);

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

struct Layer {
  LayerCells spans;
  std::string name;
  bool visible = true;
};

#endif
