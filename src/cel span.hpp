//
//  cel span.hpp
//  Animera
//
//  Created by Indiana Kernick on 25/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_cel_span_hpp
#define animera_cel_span_hpp

#include "cel.hpp"

class LayerCels {
public:
  class ConstIterator {
    friend LayerCels;
    
  public:
    ConstIterator &operator++();
    const Cel *operator*() const;
  
  private:
    std::vector<CelSpan>::const_iterator iter;
    FrameIdx idx;
  };

  LayerCels() = default;

  CelSpan *data() {
    return spans.data();
  }
  const CelSpan *data() const {
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
    spans.push_back({std::make_unique<Cel>(), len});
  }

  /// Combine consecutive null spans into a single span
  void optimize();
  
  /// Get a constant iterator to a cel
  ConstIterator find(FrameIdx) const;
  /// Get a mutable cel
  Cel *get(FrameIdx);
  /// Get a constant cel
  const Cel *get(FrameIdx) const;
  
  /// Insert a new cel after the index
  void insert(FrameIdx);
  /// Replace a cel with a new cel
  void replace(FrameIdx, bool);

  /// Extend the span at the index and shrink the following span
  /// The span will not be extended past the end
  void extend(FrameIdx);
  /// Split the span at the index into two
  /// The index becomes the first cel of the second span
  void split(FrameIdx);

  /// Replace part of a span with another span
  void replaceSpan(FrameIdx, LayerCels &);
  /// Make a copy of part of a span
  LayerCels extract(FrameIdx, FrameIdx) const;
  /// Make a truncated copy of the spans
  LayerCels truncateCopy(FrameIdx) const;

  /// Remove a cel
  void remove(FrameIdx);
  /// Replace the spans with a single null span
  void clear(FrameIdx);
  
private:
  std::vector<CelSpan> spans;
  
  explicit LayerCels(std::vector<CelSpan>);
};

struct Layer {
  LayerCels spans;
  std::string name;
  bool visible = true;
};

struct Group {
  FrameIdx end;
  std::string name;
};

class GroupArray {
public:
  std::vector<Group> &underlying() {
    return groups;
  }
  const std::vector<Group> &underlying() const {
    return groups;
  }

  void setFrames(FrameIdx);
  Group *find(FrameIdx);
  void split(FrameIdx);
  
private:
  std::vector<Group> groups;
  
  std::vector<Group>::iterator findIter(FrameIdx &);
};

#endif
