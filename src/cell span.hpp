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

/// Insert a copy of the previous cell after the index
/// If the index refers to the last cell of a span, the span is copied
/// otherwise the span is extended
void insertCopy(Spans &, FrameIdx);

/// Insert a new cell after the index
void insertNew(Spans &, FrameIdx, CellPtr);

/// Replace a cell with a new cell
void replaceNew(Spans &, FrameIdx, CellPtr);

/// Extend the span at the index and shrink the following span
/// The span will not be extended past the end
void extend(Spans &, FrameIdx);

/// Split the span at the index into two
/// The index becomes the first cell of the second span
void split(Spans &, FrameIdx);

/// Replace part of a span with another span
void replaceSpan(Spans &, FrameIdx, Spans &);

/// Make a copy of part of a span
Spans extract(const Spans &, FrameIdx, FrameIdx);

/// Make a truncated copy of the spans
Spans truncateCopy(const Spans &, FrameIdx);

/// Remove a cell
void remove(Spans &, FrameIdx);

/// Get a mutable cell
Cell *get(Spans &, FrameIdx);
/// Get a constant cell
const Cell *get(const Spans &, FrameIdx);

/// Replace the spans with a single null span
void clear(Spans &, FrameIdx);

#endif
