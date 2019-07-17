//
//  timeline.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 6/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline.hpp"

#include "serial.hpp"

namespace {

CellPtr copyCell(const CellPtr &cell) {
  return cell ? std::make_unique<Cell>(*cell) : nullptr;
}

void shrinkSpan(Spans &spans, Spans::iterator span) {
  if (--span->len <= 0) {
    spans.erase(span);
  }
}

// Get an iterator to the span at the index
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

// Insert a copy of the previous cell after the index
// If the index refers to the last cell of a span, the span is copied
// otherwise the span is extended
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

// Insert a new cell after the index
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

// Replace a cell with a new cell
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

// Extend the span at the index and shrink the following span
// The span will not be extended past the end
void extend(Spans &spans, FrameIdx idx) {
  const Spans::iterator span = findSpan(spans, idx);
  const Spans::iterator next = std::next(span);
  if (next == spans.end()) return;
  ++span->len;
  return shrinkSpan(spans, next);
}

// Remove a cell
void remove(Spans &spans, FrameIdx idx) {
  shrinkSpan(spans, findSpan(spans, idx));
}

// Get a cell
template <typename Spans>
auto get(Spans &spans, FrameIdx idx) {
  return findSpan(spans, idx)->cell.get();
}

void clear(Spans &spans, const FrameIdx len) {
  spans.clear();
  spans.push_back({nullptr, len});
}

}

Timeline::Timeline()
  : currPos{0, 0}, frameCount{0} {}

void Timeline::initDefault() {
  frameCount = 1;
  changeFrameCount();
  Layer layer;
  layer.spans.push_back({makeCell(), 1});
  layer.name = "Layer 0";
  layers.push_back(std::move(layer));
  selection = {-1, -1, -1, -1};
  changeLayerCount();
  changeFrame();
  changePos();
  Q_EMIT selectionChanged(selection);
  changeLayers(0, 1);
}

namespace {

void serializeImage(QIODevice *dev, const QImage &image) {
  assert(dev);
  assert(!image.isNull());
  image.save(dev, "png");
}

void deserializeImage(QIODevice *dev, QImage &image) {
  assert(dev);
  image.load(dev, "png");
}

}

void Timeline::serialize(QIODevice *dev) const {
  assert(dev);
  serializeBytes(dev, static_cast<uint16_t>(layers.size()));
  serializeBytes(dev, static_cast<uint16_t>(frameCount));
  for (const Layer &layer : layers) {
    serializeBytes(dev, layer.visible);
    serializeBytes(dev, static_cast<uint16_t>(layer.name.size()));
    dev->write(layer.name.data(), layer.name.size());
    serializeBytes(dev, static_cast<uint16_t>(layer.spans.size()));
    for (const CellSpan &span : layer.spans) {
      serializeBytes(dev, static_cast<uint16_t>(span.len));
      serializeBytes(dev, static_cast<bool>(span.cell));
      if (span.cell) {
        serializeImage(dev, span.cell->image);
      }
    }
  }
}

void Timeline::deserialize(QIODevice *dev) {
  assert(dev);
  layers.resize(deserializeBytesAs<uint16_t>(dev));
  frameCount = deserializeBytesAs<uint16_t>(dev);
  for (Layer &layer : layers) {
    deserializeBytes(dev, layer.visible);
    layer.name.resize(deserializeBytesAs<uint16_t>(dev));
    dev->read(layer.name.data(), layer.name.size());
    layer.spans.resize(deserializeBytesAs<uint16_t>(dev));
    for (CellSpan &span : layer.spans) {
      span.len = deserializeBytesAs<uint16_t>(dev);
      if (deserializeBytesAs<bool>(dev)) {
        span.cell = makeCell();
        deserializeImage(dev, span.cell->image);
      }
    }
  }
  selection = {-1, -1, -1, -1};
  changeFrameCount();
  changeLayerCount();
  changeFrame();
  changePos();
  Q_EMIT selectionChanged(selection);
  changeLayers(0, layerCount());
}

void Timeline::initCanvas(const Format format, const QSize size) {
  canvasFormat = format;
  canvasSize = size;
}

void Timeline::nextFrame() {
  currPos.f = (currPos.f + 1) % frameCount;
  changeFrame();
  changePos();
}

void Timeline::prevFrame() {
  currPos.f = (currPos.f - 1 + frameCount) % frameCount;
  changeFrame();
  changePos();
}

void Timeline::layerBelow() {
  currPos.l = std::min(currPos.l + 1, layerCount() - 1);
  changePos();
}

void Timeline::layerAbove() {
  currPos.l = std::max(currPos.l - 1, 0);
  changePos();
}

namespace {

CellRect normalize(const CellRect rect) {
  return {
    std::min(rect.minL, rect.maxL),
    std::min(rect.minF, rect.maxF),
    std::max(rect.minL, rect.maxL),
    std::max(rect.minF, rect.maxF)
  };
}

}

void Timeline::beginSelection() {
  selection.minL = currPos.l;
  selection.minF = currPos.f;
  selection.maxL = currPos.l;
  selection.maxF = currPos.f;
  Q_EMIT selectionChanged(selection);
}

void Timeline::continueSelection() {
  selection.maxL = currPos.l;
  selection.maxF = currPos.f;
  Q_EMIT selectionChanged(normalize(selection));
}

void Timeline::endSelection() {
  selection.maxL = currPos.l;
  selection.maxF = currPos.f;
  selection = normalize(selection);
  Q_EMIT selectionChanged(selection);
}

void Timeline::clearSelection() {
  selection = {-1, -1, -1, -1};
  Q_EMIT selectionChanged(selection);
}

void Timeline::insertLayer() {
  Layer layer;
  layer.spans.push_back({nullptr, frameCount});
  layer.name = "Layer " + std::to_string(layers.size());
  layers.insert(layers.begin() + currPos.l, std::move(layer));
  changeLayerCount();
  changeLayers(currPos.l, layerCount());
  changeFrame();
  changePos();
}

void Timeline::removeLayer() {
  if (layers.size() == 1) {
    clear(layers.front().spans, frameCount);
    layers.front().name = "Layer 0";
    layers.front().visible = true;
    changeLayers(0, 1);
  } else {
    layers.erase(layers.begin() + currPos.l);
    changeLayerCount();
    currPos.l = std::min(currPos.l, layerCount() - 1);
    changeLayers(currPos.l, layerCount());
  }
  changeFrame();
  changePos();
}

void Timeline::moveLayerUp() {
  if (currPos.l == 0) return;
  std::swap(layers[currPos.l - 1], layers[currPos.l]);
  changeLayers(currPos.l - 1, currPos.l + 1);
  changeFrame();
  layerAbove();
}

void Timeline::moveLayerDown() {
  if (currPos.l == layerCount() - 1) return;
  std::swap(layers[currPos.l], layers[currPos.l + 1]);
  changeLayers(currPos.l, currPos.l + 2);
  changeFrame();
  layerBelow();
}

void Timeline::insertFrame() {
  ++frameCount;
  changeFrameCount();
  for (LayerIdx l = 0; l != layerCount(); ++l) {
    insertCopy(layers[l].spans, currPos.f);
    changeSpan(l);
  }
  Q_EMIT selectionChanged(selection);
  nextFrame();
}

void Timeline::insertNullFrame() {
  ++frameCount;
  changeFrameCount();
  for (LayerIdx l = 0; l != layerCount(); ++l) {
    insertNew(layers[l].spans, currPos.f, nullptr);
    changeSpan(l);
  }
  Q_EMIT selectionChanged(selection);
  nextFrame();
}

void Timeline::removeFrame() {
  if (frameCount == 1) {
    for (LayerIdx l = 0; l != layerCount(); ++l) {
      clear(layers[l].spans, 1);
      changeSpan(l);
    }
  } else {
    --frameCount;
    changeFrameCount();
    for (LayerIdx l = 0; l != layerCount(); ++l) {
      remove(layers[l].spans, currPos.f);
      changeSpan(l);
    }
    Q_EMIT selectionChanged(selection);
  }
  currPos.f = std::max(currPos.f - 1, 0);
  changeFrame();
  changePos();
}

void Timeline::clearCell() {
  replaceNew(layers[currPos.l].spans, currPos.f, nullptr);
  changeSpan(currPos.l);
  changeFrame();
  changePos();
}

void Timeline::extendCell() {
  extend(layers[currPos.l].spans, currPos.f);
  changeSpan(currPos.l);
  nextFrame();
}

void Timeline::requestCell() {
  replaceNew(layers[currPos.l].spans, currPos.f, makeCell());
  changeSpan(currPos.l);
  changeFrame();
  changePos();
}

void Timeline::setCurrPos(const CellPos pos) {
  assert(0 <= pos.l);
  assert(pos.l < layerCount());
  assert(0 <= pos.f);
  assert(pos.f < frameCount);
  if (currPos.f != pos.f) {
    currPos = pos;
    changeFrame();
    changePos();
  } else if (currPos.l != pos.l) {
    currPos.l = pos.l;
    changePos();
  }
}

void Timeline::setVisibility(const LayerIdx idx, const bool visible) {
  assert(0 <= idx);
  assert(idx < layerCount());
  bool &layerVis = layers[idx].visible;
  // if (layerVis != visible) {
     layerVis = visible;
  //   Q_EMIT visibilityChanged(idx, visible);
  // }
  changeFrame();
}

void Timeline::setName(const LayerIdx idx, const std::string_view name) {
  assert(0 <= idx);
  assert(idx < layerCount());
  layers[idx].name = name;
  // Q_EMIT nameChanged(idx, name);
}

CellPtr Timeline::makeCell() const {
  return std::make_unique<Cell>(canvasSize, canvasFormat);
}

Cell *Timeline::getCell(const CellPos pos) {
  return get(layers[pos.l].spans, pos.f);
}

Frame Timeline::getFrame(const FrameIdx pos) {
  Frame frame;
  frame.reserve(layers.size());
  for (const Layer &layer : layers) {
    if (layer.visible) {
      const Cell *cell = get(layer.spans, pos);
      if (cell) {
        frame.push_back(cell);
      }
    }
  }
  return frame;
}

LayerIdx Timeline::layerCount() const {
  return static_cast<LayerIdx>(layers.size());
}

void Timeline::changePos() {
  Q_EMIT currPosChanged(currPos);
  Q_EMIT currCellChanged(getCell(currPos));
}

void Timeline::changeFrame() {
  Q_EMIT frameChanged(getFrame(currPos.f));
}

void Timeline::changeSpan(const LayerIdx idx) {
  Q_EMIT layerChanged(idx, layers[idx].spans);
}

void Timeline::changeLayers(const LayerIdx begin, const LayerIdx end) {
  assert(begin < end);
  for (LayerIdx l = begin; l != end; ++l) {
    Q_EMIT layerChanged(l, layers[l].spans);
    Q_EMIT visibilityChanged(l, layers[l].visible);
    Q_EMIT nameChanged(l, layers[l].name);
  }
}

void Timeline::changeFrameCount() {
  Q_EMIT frameCountChanged(frameCount);
}

void Timeline::changeLayerCount() {
  Q_EMIT layerCountChanged(layerCount());
}

#include "timeline.moc"
