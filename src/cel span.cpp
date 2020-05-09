//
//  cel span.cpp
//  Animera
//
//  Created by Indiana Kernick on 25/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "cel span.hpp"

namespace {

using Spans = std::vector<CelSpan>;

FrameIdx spanLength(const Spans &spans) {
  FrameIdx len{0};
  for (const CelSpan &span : spans) {
    assert(span.len >= FrameIdx{0});
    len += span.len;
  }
  return len;
}

void shrinkSpan(Spans &spans, Spans::iterator span) {
  if (--span->len <= FrameIdx{0}) {
    spans.erase(span);
  }
}

template <typename Spans>
auto findSpan(Spans &spans, FrameIdx &idx) {
  assert(FrameIdx{0} <= idx);
  for (auto s = spans.begin(); s != spans.end(); ++s) {
    if (idx < s->len) {
      return s;
    }
    idx -= s->len;
  }
  Q_UNREACHABLE();
}

}

LayerCels::ConstIterator &LayerCels::ConstIterator::operator++() {
  assert(FrameIdx{0} < iter->len);
  assert(idx < iter->len);
  if (++idx == iter->len) {
    ++iter;
    idx = FrameIdx{0};
  }
  return *this;
}

const Cel *LayerCels::ConstIterator::operator*() const {
  return iter->cel.get();
}
         
void LayerCels::optimize() {
  FrameIdx prevNull{0};
  for (auto s = spans.begin(); s != spans.end(); ++s) {
    if (s->cel->isNull()) {
      if (prevNull > FrameIdx{0}) {
        auto prev = std::prev(s);
        s->len += prev->len;
        s = spans.erase(prev);
      }
      prevNull = s->len;
    } else {
      prevNull = {0};
    }
  }
}

LayerCels::ConstIterator LayerCels::find(FrameIdx idx) const {
  ConstIterator iter;
  iter.iter = findSpan(spans, idx);;
  iter.idx = idx;
  return iter;
}

Cel *LayerCels::get(FrameIdx idx) {
  return const_cast<Cel *>(std::as_const(*this).get(idx));
}

const Cel *LayerCels::get(FrameIdx idx) const {
  return findSpan(spans, idx)->cel.get();
}

void LayerCels::insert(FrameIdx idx) {
  Spans::iterator span = findSpan(spans, idx);
  if (span->cel->isNull()) {
    ++span->len;
  } else if (idx < span->len - FrameIdx{1}) {
    const FrameIdx leftSize = idx;
    const FrameIdx rightSize = span->len - idx;
    span->len = leftSize;
    CelPtr copy = std::make_unique<Cel>(*span->cel);
    span = spans.insert(++span, {std::make_unique<Cel>()});
    spans.insert(++span, {std::move(copy), rightSize});
  } else {
    spans.insert(++span, {std::make_unique<Cel>()});
  }
}

void LayerCels::replace(FrameIdx idx, const bool isolate) {
  Spans::iterator span = findSpan(spans, idx);
  if (!isolate && span->cel->isNull()) return;
  if (span->len == FrameIdx{1}) {
    *span->cel = {};
    return;
  }
  const FrameIdx leftSize = idx;
  const FrameIdx rightSize = span->len - idx - FrameIdx{1};
  if (leftSize == FrameIdx{0}) {
    span->len = rightSize;
    spans.insert(span, {std::make_unique<Cel>()});
    return;
  } else if (rightSize == FrameIdx{0}) {
    span->len = leftSize;
    spans.insert(++span, {std::make_unique<Cel>()});
    return;
  }
  span->len = leftSize;
  CelPtr copy = std::make_unique<Cel>(*span->cel);
  span = spans.insert(++span, {std::make_unique<Cel>()});
  spans.insert(++span, {std::move(copy), rightSize});
}

void LayerCels::extend(FrameIdx idx) {
  const Spans::iterator span = findSpan(spans, idx);
  const Spans::iterator next = std::next(span);
  if (next == spans.end()) return;
  ++span->len;
  return shrinkSpan(spans, next);
}

void LayerCels::split(FrameIdx idx) {
  Spans::iterator span = findSpan(spans, idx);
  if (idx == FrameIdx{0}) return;
  const FrameIdx leftSize = idx;
  const FrameIdx rightSize = span->len - idx;
  span->len = leftSize;
  CelPtr copy = std::make_unique<Cel>(*span->cel);
  spans.insert(++span, {std::move(copy), rightSize});
}

namespace {

Spans::iterator insertSpan(Spans &dst, Spans::iterator pos, Spans &src) {
  return dst.insert(
    pos,
    std::make_move_iterator(src.begin()),
    std::make_move_iterator(src.end())
  );
}

void removeSpan(Spans &spans, Spans::iterator span, FrameIdx len) {
  assert(span != spans.end());
  while (len > FrameIdx{0} && span->len <= len) {
    len -= span->len;
    span = spans.erase(span);
  }
  if (span->len > len) {
    span->len = span->len - len;
  }
}

}

void LayerCels::replaceSpan(FrameIdx idx, LayerCels &newCels) {
  Spans &newSpans = newCels.spans;
  const FrameIdx len = spanLength(newSpans);
  if (len <= FrameIdx{0}) return;
  Spans::iterator span = findSpan(spans, idx);
  if (idx == FrameIdx{0}) {
    if (span->len < len) {
      span = insertSpan(spans, span, newSpans) + newSpans.size();
      removeSpan(spans, span, len - span->len);
    } else if (span->len > len) {
      span->len -= len;
      insertSpan(spans, span, newSpans);
    } else {
      span = spans.erase(span);
      insertSpan(spans, span, newSpans);
    }
  } else {
    const FrameIdx leftSize = idx;
    const FrameIdx rightSize = span->len - idx - len;
    span->len = leftSize;
    if (rightSize < FrameIdx{0}) {
      span = insertSpan(spans, ++span, newSpans) + newSpans.size();
      removeSpan(spans, span, -rightSize);
    } else if (rightSize > FrameIdx{0}) {
      CelPtr copy = std::make_unique<Cel>(*span->cel);
      span = insertSpan(spans, ++span, newSpans) + newSpans.size();
      span = spans.insert(span, {std::move(copy), rightSize});
    } else {
      insertSpan(spans, ++span, newSpans);
    }
  }
}

LayerCels LayerCels::extract(FrameIdx idx, FrameIdx len) const {
  Spans newSpans;
  if (len <= FrameIdx{0}) return LayerCels{std::move(newSpans)};
  Spans::const_iterator span = findSpan(spans, idx);
  const FrameIdx leftSize = idx;
  const FrameIdx rightSize = span->len - idx - len;
  if (rightSize >= FrameIdx{0}) {
    newSpans.push_back({std::make_unique<Cel>(*span->cel), len});
  } else {
    newSpans.push_back({std::make_unique<Cel>(*span->cel), span->len - leftSize});
    len -= span->len - leftSize;
    ++span;
    while (len > span->len) {
      newSpans.push_back({std::make_unique<Cel>(*span->cel), span->len});
      len -= span->len;
      ++span;
    }
    if (len > FrameIdx{0}) {
      newSpans.push_back({std::make_unique<Cel>(*span->cel), len});
    }
  }
  return LayerCels{std::move(newSpans)};
}

LayerCels LayerCels::truncateCopy(FrameIdx len) const {
  Spans newSpans;
  Spans::const_iterator span = spans.begin();
  while (span != spans.end() && len > span->len) {
    newSpans.push_back({std::make_unique<Cel>(*span->cel), span->len});
    len -= span->len;
    ++span;
  }
  if (span != spans.end() && len > FrameIdx{0}) {
    newSpans.push_back({std::make_unique<Cel>(*span->cel), len});
  }
  return LayerCels{std::move(newSpans)};
}

void LayerCels::remove(FrameIdx idx) {
  shrinkSpan(spans, findSpan(spans, idx));
}

void LayerCels::clear(const FrameIdx len) {
  spans.clear();
  spans.push_back({std::make_unique<Cel>(), len});
}

LayerCels::LayerCels(std::vector<CelSpan> spans)
  : spans{std::move(spans)} {}
