//
//  cell span.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 25/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cell span.hpp"

namespace {

CellPtr copyCell(const CellPtr &cell) {
  return cell ? std::make_unique<Cell>(*cell) : nullptr;
}

FrameIdx spanLength(const Spans &spans) {
  FrameIdx len = 0;
  for (const CellSpan &span : spans) {
    assert(span.len >= 0);
    len += span.len;
  }
  return len;
}

void shrinkSpan(Spans &spans, Spans::iterator span) {
  if (--span->len <= 0) {
    spans.erase(span);
  }
}

template <typename Spans>
auto findSpan(Spans &spans, FrameIdx &idx) {
  assert(0 <= idx);
  for (auto s = spans.begin(); s != spans.end(); ++s) {
    if (idx < s->len) {
      return s;
    }
    idx -= s->len;
  }
  Q_UNREACHABLE();
}

}

void insertCopy(Spans &spans, FrameIdx idx) {
  Spans::iterator span = findSpan(spans, idx);
  if (idx < span->len - 1) {
    ++span->len;
  } else {
    if (span->cell) {
      auto copy = std::make_unique<Cell>(*span->cell);
      spans.insert(++span, {std::move(copy)});
    } else {
      ++span->len;
    }
  }
}

void insertNew(Spans &spans, FrameIdx idx, CellPtr cell) {
  Spans::iterator span = findSpan(spans, idx);
  if (span->cell == cell) {
    ++span->len;
  } else if (idx < span->len - 1) {
    const FrameIdx leftSize = idx;
    const FrameIdx rightSize = span->len - idx;
    span->len = leftSize;
    auto copy = std::make_unique<Cell>(*span->cell);
    span = spans.insert(++span, {std::move(cell)});
    spans.insert(++span, {std::move(copy), rightSize});
  } else {
    spans.insert(++span, {std::move(cell)});
  }
}

void replaceNew(Spans &spans, FrameIdx idx, CellPtr cell) {
  Spans::iterator span = findSpan(spans, idx);
  if (span->cell == cell) return;
  if (span->len == 1) {
    span->cell = std::move(cell);
    return;
  }
  const FrameIdx leftSize = idx;
  const FrameIdx rightSize = span->len - idx - 1;
  if (leftSize == 0) {
    span->len = rightSize;
    spans.insert(span, {std::move(cell)});
    return;
  } else if (rightSize == 0) {
    span->len = leftSize;
    spans.insert(++span, {std::move(cell)});
    return;
  }
  span->len = leftSize;
  auto copy = copyCell(span->cell);
  span = spans.insert(++span, {std::move(cell)});
  spans.insert(++span, {std::move(copy), rightSize});
}

void extend(Spans &spans, FrameIdx idx) {
  const Spans::iterator span = findSpan(spans, idx);
  const Spans::iterator next = std::next(span);
  if (next == spans.end()) return;
  ++span->len;
  return shrinkSpan(spans, next);
}

void split(Spans &spans, FrameIdx idx) {
  Spans::iterator span = findSpan(spans, idx);
  if (idx == 0) return;
  const FrameIdx leftSize = idx;
  const FrameIdx rightSize = span->len - idx;
  span->len = leftSize;
  CellPtr copy = copyCell(span->cell);
  spans.insert(++span, {std::move(copy), rightSize});
}

namespace {

Spans::iterator insert(Spans &dst, Spans::iterator pos, Spans &src) {
  return dst.insert(
    pos,
    std::make_move_iterator(src.begin()),
    std::make_move_iterator(src.end())
  );
}

void removeSpan(Spans &spans, Spans::iterator span, FrameIdx len) {
  assert(span != spans.end());
  while (len > 0 && span->len <= len) {
    len -= span->len;
    span = spans.erase(span);
  }
  if (span->len > len) {
    span->len = span->len - len;
  }
}

}

void replaceSpan(Spans &spans, FrameIdx idx, Spans &newSpans) {
  const FrameIdx len = spanLength(newSpans);
  if (len <= 0) return;
  Spans::iterator span = findSpan(spans, idx);
  if (idx == 0) {
    if (span->len < len) {
      span = insert(spans, span, newSpans) + newSpans.size();
      removeSpan(spans, span, len - span->len);
    } else if (span->len > len) {
      CellPtr copy = std::move(span->cell);
      span = insert(spans, span, newSpans) + newSpans.size();
      spans.insert(span, {std::move(copy), span->len - len});
    } else {
      span = spans.erase(span);
      insert(spans, span, newSpans);
    }
  } else {
    const FrameIdx leftSize = idx;
    const FrameIdx rightSize = span->len - idx - len;
    span->len = leftSize;
    if (rightSize < 0) {
      span = insert(spans, ++span, newSpans) + newSpans.size();
      removeSpan(spans, span, -rightSize);
    } else if (rightSize > 0) {
      CellPtr copy = copyCell(span->cell);
      span = insert(spans, ++span, newSpans) + newSpans.size();
      span = spans.insert(span, {std::move(copy), rightSize});
    } else {
      insert(spans, ++span, newSpans);
    }
  }
}

Spans extract(const Spans &spans, FrameIdx idx, FrameIdx len) {
  Spans newSpans;
  if (len <= 0) return newSpans;
  Spans::const_iterator span = findSpan(spans, idx);
  const FrameIdx leftSize = idx;
  const FrameIdx rightSize = span->len - idx - len;
  if (rightSize >= 0) {
    newSpans.push_back({copyCell(span->cell), len});
  } else {
    newSpans.push_back({copyCell(span->cell), span->len - leftSize});
    len -= span->len - leftSize;
    ++span;
    while (len > span->len) {
      newSpans.push_back({copyCell(span->cell), span->len});
      len -= span->len;
      ++span;
    }
    if (len > 0) {
      newSpans.push_back({copyCell(span->cell), len});
    }
  }
  return newSpans;
}

Spans truncateCopy(const Spans &spans, FrameIdx len) {
  Spans newSpans;
  Spans::const_iterator span = spans.begin();
  while (span != spans.end() && len > span->len) {
    newSpans.push_back({copyCell(span->cell), span->len});
    len -= span->len;
    ++span;
  }
  if (span != spans.end() && len > 0) {
    newSpans.push_back({copyCell(span->cell), len});
  }
  return newSpans;
}

void remove(Spans &spans, FrameIdx idx) {
  shrinkSpan(spans, findSpan(spans, idx));
}

Cell *get(Spans &spans, FrameIdx idx) {
  return findSpan(spans, idx)->cell.get();
}

const Cell *get(const Spans &spans, FrameIdx idx) {
  return findSpan(spans, idx)->cell.get();
}

void clear(Spans &spans, const FrameIdx len) {
  spans.clear();
  spans.push_back({nullptr, len});
}
