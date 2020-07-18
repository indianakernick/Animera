//
//  cel array.cpp
//  Animera
//
//  Created by Indiana Kernick on 25/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "cel array.hpp"

namespace {

FrameIdx arrayLength(const std::vector<Cel> &cels) {
  FrameIdx len{0};
  for (const Cel &cel : cels) {
    assert(cel.len >= FrameIdx{0});
    len += cel.len;
  }
  return len;
}

void shrinkCel(std::vector<Cel> &cels, std::vector<Cel>::iterator cel) {
  if (--cel->len <= FrameIdx{0}) {
    cels.erase(cel);
  }
}

template <typename Cels>
auto findCelIter(Cels &cels, FrameIdx &idx) {
  assert(FrameIdx{0} <= idx);
  for (auto s = cels.begin(); s != cels.end(); ++s) {
    if (idx < s->len) {
      return s;
    }
    idx -= s->len;
  }
  Q_UNREACHABLE();
}

}

CelIterator::CelIterator(tcb::span<const Cel> cels, FrameIdx idx)
  : iter{findCelIter(cels, idx)}, idx{idx} {}

const CelImage *CelIterator::img() const {
  return iter->cel.get();
}

bool CelIterator::incr() {
  assert(FrameIdx{0} < iter->len);
  assert(idx < iter->len);
  if (++idx == iter->len) {
    ++iter;
    idx = FrameIdx{0};
    return true;
  } else {
    return false;
  }
}


void optimizeCelArray(std::vector<Cel> &cels) {
  FrameIdx prevNull{0};
  for (auto s = cels.begin(); s != cels.end(); ++s) {
    if (s->cel->isNull()) {
      if (prevNull > FrameIdx{0}) {
        auto prev = std::prev(s);
        s->len += prev->len;
        s = cels.erase(prev);
      }
      prevNull = s->len;
    } else {
      prevNull = FrameIdx{0};
    }
  }
}

CelImage *getImage(std::vector<Cel> &cels, const FrameIdx idx) {
  return const_cast<CelImage *>(getImage(tcb::span<const Cel>{cels}, idx));
}

const CelImage *getImage(const tcb::span<const Cel> cels, FrameIdx idx) {
  return findCelIter(cels, idx)->cel.get();
}

void insertCelFrame(std::vector<Cel> &cels, FrameIdx idx) {
  auto iter = findCelIter(cels, idx);
  if (iter->cel->isNull()) {
    ++iter->len;
  } else if (idx < iter->len - FrameIdx{1}) {
    const FrameIdx leftSize = idx;
    const FrameIdx rightSize = iter->len - idx;
    iter->len = leftSize;
    CelImagePtr copy = std::make_unique<CelImage>(*iter->cel);
    iter = cels.insert(++iter, {std::make_unique<CelImage>(), FrameIdx{1}});
    cels.insert(++iter, {std::move(copy), rightSize});
  } else {
    cels.insert(++iter, {std::make_unique<CelImage>(), FrameIdx{1}});
  }
}

void replaceCelFrame(std::vector<Cel> &cels, FrameIdx idx, const bool isolate) {
  auto iter = findCelIter(cels, idx);
  if (!isolate && iter->cel->isNull()) return;
  if (iter->len == FrameIdx{1}) {
    *iter->cel = {};
    return;
  }
  const FrameIdx leftSize = idx;
  const FrameIdx rightSize = iter->len - idx - FrameIdx{1};
  if (leftSize == FrameIdx{0}) {
    iter->len = rightSize;
    cels.insert(iter, {std::make_unique<CelImage>(), FrameIdx{1}});
    return;
  } else if (rightSize == FrameIdx{0}) {
    iter->len = leftSize;
    cels.insert(++iter, {std::make_unique<CelImage>(), FrameIdx{1}});
    return;
  }
  iter->len = leftSize;
  CelImagePtr copy = std::make_unique<CelImage>(*iter->cel);
  iter = cels.insert(++iter, {std::make_unique<CelImage>(), FrameIdx{1}});
  cels.insert(++iter, {std::move(copy), rightSize});
}

void extendCel(std::vector<Cel> &cels, FrameIdx idx) {
  const auto iter = findCelIter(cels, idx);
  const auto next = std::next(iter);
  if (next == cels.end()) return;
  ++iter->len;
  return shrinkCel(cels, next);
}

void splitCel(std::vector<Cel> &cels, FrameIdx idx) {
  auto iter = findCelIter(cels, idx);
  if (idx == FrameIdx{0}) return;
  const FrameIdx leftSize = idx;
  const FrameIdx rightSize = iter->len - idx;
  iter->len = leftSize;
  CelImagePtr copy = std::make_unique<CelImage>(*iter->cel);
  cels.insert(++iter, {std::move(copy), rightSize});
}

namespace {

std::vector<Cel>::iterator insertCels(std::vector<Cel> &dst, std::vector<Cel>::iterator pos, std::vector<Cel> &src) {
  return dst.insert(
    pos,
    std::make_move_iterator(src.begin()),
    std::make_move_iterator(src.end())
  );
}

void removeCels(std::vector<Cel> &cels, std::vector<Cel>::iterator iter, FrameIdx len) {
  assert(iter != cels.end());
  while (len > FrameIdx{0} && iter->len <= len) {
    len -= iter->len;
    iter = cels.erase(iter);
  }
  if (iter->len > len) {
    iter->len = iter->len - len;
  }
}

}

void replaceCelArray(std::vector<Cel> &cels, FrameIdx idx, std::vector<Cel> &newCels) {
  const FrameIdx len = arrayLength(newCels);
  if (len <= FrameIdx{0}) return;
  auto iter = findCelIter(cels, idx);
  if (idx == FrameIdx{0}) {
    if (iter->len < len) {
      iter = insertCels(cels, iter, newCels) + newCels.size();
      removeCels(cels, iter, len - iter->len);
    } else if (iter->len > len) {
      iter->len -= len;
      insertCels(cels, iter, newCels);
    } else {
      iter = cels.erase(iter);
      insertCels(cels, iter, newCels);
    }
  } else {
    const FrameIdx leftSize = idx;
    const FrameIdx rightSize = iter->len - idx - len;
    iter->len = leftSize;
    if (rightSize < FrameIdx{0}) {
      iter = insertCels(cels, ++iter, newCels) + newCels.size();
      removeCels(cels, iter, -rightSize);
    } else if (rightSize > FrameIdx{0}) {
      CelImagePtr copy = std::make_unique<CelImage>(*iter->cel);
      iter = insertCels(cels, ++iter, newCels) + newCels.size();
      iter = cels.insert(iter, {std::move(copy), rightSize});
    } else {
      insertCels(cels, ++iter, newCels);
    }
  }
}

std::vector<Cel> extractCelArray(tcb::span<const Cel> cels, FrameIdx idx, FrameIdx len) {
  std::vector<Cel> newCels;
  if (len <= FrameIdx{0}) return newCels;
  auto cel = findCelIter(cels, idx);
  const FrameIdx leftSize = idx;
  const FrameIdx rightSize = cel->len - idx - len;
  if (rightSize >= FrameIdx{0}) {
    newCels.push_back({std::make_unique<CelImage>(*cel->cel), len});
  } else {
    newCels.push_back({std::make_unique<CelImage>(*cel->cel), cel->len - leftSize});
    len -= cel->len - leftSize;
    ++cel;
    while (len > cel->len) {
      newCels.push_back({std::make_unique<CelImage>(*cel->cel), cel->len});
      len -= cel->len;
      ++cel;
    }
    if (len > FrameIdx{0}) {
      newCels.push_back({std::make_unique<CelImage>(*cel->cel), len});
    }
  }
  return newCels;
}

std::vector<Cel> truncateCopyCelArray(tcb::span<const Cel> cels, FrameIdx len) {
  std::vector<Cel> newCels;
  auto iter = cels.begin();
  while (iter != cels.end() && len > iter->len) {
    newCels.push_back({std::make_unique<CelImage>(*iter->cel), iter->len});
    len -= iter->len;
    ++iter;
  }
  if (iter != cels.end() && len > FrameIdx{0}) {
    newCels.push_back({std::make_unique<CelImage>(*iter->cel), len});
  }
  return newCels;
}

void removeCelFrame(std::vector<Cel> &cels, FrameIdx idx) {
  shrinkCel(cels, findCelIter(cels, idx));
}

void clearCelArray(std::vector<Cel> &cels, const FrameIdx len) {
  cels.clear();
  cels.push_back({std::make_unique<CelImage>(), len});
}
