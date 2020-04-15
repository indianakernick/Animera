//
//  timeline.cpp
//  Animera
//
//  Created by Indiana Kernick on 6/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "timeline.hpp"

#include "file io.hpp"
#include "composite.hpp"
#include "scope time.hpp"
#include "export png.hpp"
#include "sprite file.hpp"
#include "sprite export.hpp"

Timeline::Timeline()
  : pos{LayerIdx{0}, FrameIdx{0}}, frameCount{0} {}

void Timeline::initDefault() {
  frameCount = FrameIdx{1};
  Layer layer;
  layer.spans.pushNull(frameCount);
  layer.name = "Layer 0";
  layers.push_back(std::move(layer));
  selection = empty_rect;
  delay = ctrl_delay.def;
  change();
}

void Timeline::optimize() {
  LayerIdx idx{};
  for (Layer &layer : layers) {
    for (CellSpan &span : layer.spans) {
      ::shrinkCell(*span.cell, toRect(canvasSize));
    }
    layer.spans.optimize();
    changeSpan(idx);
    ++idx;
  }
  changeFrame();
  changePos();
}

void Timeline::change() {
  changeFrameCount();
  changeLayerCount();
  changeFrame();
  changeCell();
  changePos();
  Q_EMIT selectionChanged(selection);
  changeLayers(LayerIdx{0}, layerCount());
  Q_EMIT delayChanged(delay);
}

Error Timeline::openImage(
  const QString &path,
  const PaletteSpan palette,
  Format &format,
  QSize &size
) {
  QImage image;
  FileReader reader;
  TRY(reader.open(path));
  TRY(importSpritePng(reader.dev(), palette, image, format));
  TRY(reader.flush());
  canvasFormat = format;
  canvasSize = size = image.size();
  
  frameCount = FrameIdx{1};
  Layer layer;
  layer.spans.pushNull(frameCount);
  layer.spans.begin()->cell->img = std::move(image);
  layer.name = "Layer 0";
  layers.push_back(std::move(layer));
  selection = empty_rect;
  delay = ctrl_delay.def;
  return {};
}

Error Timeline::importImage(const QString &path) {
  QImage image;
  FileReader reader;
  TRY(reader.open(path));
  TRY(importCellPng(reader.dev(), image, canvasFormat));
  TRY(reader.flush());
  
  Cell *cell = getCell(pos);
  cell->pos = {};
  cell->img = std::move(image);
  
  changeFrame();
  changeCell();
  changeSpan(pos.l);
  Q_EMIT modified();
  return {};
}

Error Timeline::serializeHead(QIODevice &dev) const {
  SCOPE_TIME("Timeline::serializeHead");

  SpriteInfo info;
  info.width = canvasSize.width();
  info.height = canvasSize.height();
  info.layers = layerCount();
  info.frames = frameCount;
  info.delay = delay;
  info.format = canvasFormat;
  return writeAHDR(dev, info);
}

Error Timeline::serializeBody(QIODevice &dev) const {
  SCOPE_TIME("Timeline::serializeBody");

  for (const Layer &layer : layers) {
    TRY(writeLHDR(dev, layer));
    for (const CellSpan &span : layer.spans) {
      TRY(writeCHDR(dev, span));
      if (*span.cell) TRY(writeCDAT(dev, span.cell->img, canvasFormat));
    }
  }
  return {};
}

Error Timeline::serializeTail(QIODevice &dev) const {
  SCOPE_TIME("Timeline::serializeTail");

  return writeAEND(dev);
}

Error Timeline::deserializeHead(QIODevice &dev, Format &format, QSize &size) {
  SCOPE_TIME("Timeline::deserializeHead");

  SpriteInfo info;
  TRY(readAHDR(dev, info));
  canvasSize = size = {info.width, info.height};
  layers.resize(+info.layers);
  frameCount = info.frames;
  canvasFormat = format = info.format;
  delay = info.delay;
  return {};
}

Error Timeline::deserializeBody(QIODevice &dev) {
  SCOPE_TIME("Timeline::deserializeBody");

  for (Layer &layer : layers) {
    TRY(readLHDR(dev, layer));
    for (CellSpan &span : layer.spans) {
      TRY(readCHDR(dev, span, canvasFormat));
      if (*span.cell) TRY(readCDAT(dev, span.cell->img, canvasFormat));
    }
  }
  return {};
}

Error Timeline::deserializeTail(QIODevice &dev) {
  SCOPE_TIME("Timeline::deserializeTail");

  TRY(readAEND(dev));
  selection = empty_rect;
  change();
  return {};
}

Error Timeline::exportTimeline(const ExportOptions &options, const PaletteCSpan palette) const {
  Exporter exporter{options, palette, canvasFormat, canvasSize};
  return exporter.exportSprite(layers);
}

LayerIdx Timeline::getLayers() const {
  return layerCount();
}

FrameIdx Timeline::getFrames() const {
  return frameCount;
}

CellPos Timeline::getPos() const {
  return pos;
}

CellRect Timeline::getSelection() const {
  return selection;
}

void Timeline::initCanvas(const Format format, const QSize size) {
  canvasFormat = format;
  canvasSize = size;
}

void Timeline::nextFrame() {
  if (locked) return;
  pos.f = (pos.f + FrameIdx{1}) % frameCount;
  if (frameCount == FrameIdx{1}) return;
  changeFrame();
  changeCell();
  changePos();
}

void Timeline::prevFrame() {
  if (locked) return;
  pos.f = (pos.f - FrameIdx{1} + frameCount) % frameCount;
  if (frameCount == FrameIdx{1}) return;
  changeFrame();
  changeCell();
  changePos();
}

void Timeline::layerBelow() {
  if (locked) return;
  pos.l = std::min(pos.l + LayerIdx{1}, layerCount() - LayerIdx{1});
  changePos();
}

void Timeline::layerAbove() {
  if (locked) return;
  pos.l = std::max(pos.l - LayerIdx{1}, LayerIdx{0});
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
  if (locked) return;
  selection.minL = pos.l;
  selection.minF = pos.f;
  selection.maxL = pos.l;
  selection.maxF = pos.f;
  Q_EMIT selectionChanged(selection);
}

void Timeline::continueSelection() {
  if (locked) return;
  selection.maxL = pos.l;
  selection.maxF = pos.f;
  Q_EMIT selectionChanged(normalize(selection));
}

void Timeline::endSelection() {
  if (locked) return;
  selection.maxL = pos.l;
  selection.maxF = pos.f;
  selection = normalize(selection);
  Q_EMIT selectionChanged(selection);
}

void Timeline::clearSelection() {
  if (locked) return;
  selection = empty_rect;
  Q_EMIT selectionChanged(selection);
}

void Timeline::insertLayer() {
  if (locked) return;
  Layer layer;
  layer.spans.pushNull(frameCount);
  layer.name = "Layer " + std::to_string(layers.size());
  layers.insert(layers.begin() + +pos.l, std::move(layer));
  changeLayerCount();
  Q_EMIT selectionChanged(selection);
  changeLayers(pos.l, layerCount());
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::removeLayer() {
  if (locked) return;
  const QRect rect = getCell(pos)->rect();
  if (layers.size() == 1) {
    layers.front().spans.clear(frameCount);
    layers.front().name = "Layer 0";
    layers.front().visible = true;
    changeLayers(LayerIdx{0}, LayerIdx{1});
  } else {
    layers.erase(layers.begin() + +pos.l);
    changeLayerCount();
    Q_EMIT selectionChanged(selection);
    pos.l = std::min(pos.l, layerCount() - LayerIdx{1});
    changeLayers(pos.l, layerCount());
  }
  changeFrame();
  changeCell(rect);
  changePos();
  Q_EMIT modified();
}

void Timeline::moveLayerUp() {
  if (locked) return;
  if (pos.l == LayerIdx{0}) return;
  std::swap(layers[+(pos.l - LayerIdx{1})], layers[+pos.l]);
  changeLayers(pos.l - LayerIdx{1}, pos.l + LayerIdx{1});
  changeFrame();
  const QRect upperRect = getCell({pos.l - LayerIdx{1}, pos.f})->rect();
  const QRect lowerRect = getCell(pos)->rect();
  changeCell(upperRect.united(lowerRect));
  layerAbove();
  Q_EMIT modified();
}

void Timeline::moveLayerDown() {
  if (locked) return;
  if (pos.l == layerCount() - LayerIdx{1}) return;
  std::swap(layers[+pos.l], layers[+(pos.l + LayerIdx{1})]);
  changeLayers(pos.l, pos.l + LayerIdx{2});
  changeFrame();
  const QRect upperRect = getCell(pos)->rect();
  const QRect lowerRect = getCell({pos.l + LayerIdx{1}, pos.f})->rect();
  changeCell(upperRect.united(lowerRect));
  layerBelow();
  Q_EMIT modified();
}

void Timeline::insertFrame() {
  if (locked) return;
  ++frameCount;
  changeFrameCount();
  for (LayerIdx l = {}; l != layerCount(); ++l) {
    layers[+l].spans.insert(pos.f);
    changeSpan(l);
  }
  Q_EMIT selectionChanged(selection);
  nextFrame();
  Q_EMIT modified();
}

void Timeline::removeFrame() {
  if (locked) return;
  if (frameCount == FrameIdx{1}) {
    for (LayerIdx l = {}; l != layerCount(); ++l) {
      layers[+l].spans.clear(FrameIdx{1});
      changeSpan(l);
    }
  } else {
    --frameCount;
    changeFrameCount();
    for (LayerIdx l = {}; l != layerCount(); ++l) {
      layers[+l].spans.remove(pos.f);
      changeSpan(l);
    }
    Q_EMIT selectionChanged(selection);
  }
  pos.f = std::max(pos.f - FrameIdx{1}, FrameIdx{0});
  changeFrame();
  changeCell();
  changePos();
  Q_EMIT modified();
}

void Timeline::clearCell() {
  if (locked) return;
  Layer &layer = layers[+pos.l];
  const QRect rect = layer.spans.get(pos.f)->rect();
  layer.spans.replace(pos.f, false);
  changeSpan(pos.l);
  changeFrame();
  changeCell(rect);
  changePos();
  Q_EMIT modified();
}

void Timeline::extendCell() {
  if (locked) return;
  layers[+pos.l].spans.extend(pos.f);
  changeSpan(pos.l);
  nextFrame();
  Q_EMIT modified();
}

void Timeline::splitCell() {
  if (locked) return;
  layers[+pos.l].spans.split(pos.f);
  changeSpan(pos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::growCell(const QRect rect) {
  Cell &cell = *getCell(pos);
  if (cell) {
    ::growCell(cell, canvasFormat, rect);
    return;
  }
  if (locked) return;
  layers[+pos.l].spans.replace(pos.f, true);
  ::growCell(*getCell(pos), canvasFormat, rect);
  changeSpan(pos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::shrinkCell(const QRect rect) {
  if (locked) return;
  Cell &cell = *getCell(pos);
  if (!cell) return;
  ::shrinkCell(cell, rect);
  if (!cell) {
    layers[+pos.l].spans.optimize();
    changeSpan(pos.l);
    changeFrame();
    changePos();
  }
  Q_EMIT modified();
}

void Timeline::setPos(const CellPos newPos) {
  assert(LayerIdx{0} <= newPos.l);
  assert(newPos.l < layerCount());
  assert(FrameIdx{0} <= newPos.f);
  assert(newPos.f < frameCount);
  if (locked) return;
  if (pos.f != newPos.f) {
    pos = newPos;
    changeFrame();
    changeCell();
    changePos();
  } else if (pos.l != newPos.l) {
    pos.l = newPos.l;
    changePos();
  }
}

void Timeline::setVisibility(const LayerIdx idx, const bool visible) {
  assert(LayerIdx{0} <= idx);
  assert(idx < layerCount());
  Layer &layer = layers[+idx];
  if (layer.visible != visible) {
    layer.visible = visible;
    Q_EMIT visibilityChanged(idx, visible);
    changeFrame();
    changeCell(layer.spans.get(pos.f)->rect());
    Q_EMIT modified();
  }
}

void Timeline::isolateVisibility(const LayerIdx idx) {
  assert(LayerIdx{0} <= idx);
  assert(idx < layerCount());
  
  QRect cellChanged;
  for (LayerIdx l = {}; l != layerCount(); ++l) {
    Layer &layer = layers[+l];
    if ((l != idx) == layer.visible) {
      layer.visible = !layer.visible;
      Q_EMIT visibilityChanged(l, layer.visible);
      cellChanged = cellChanged.united(layer.spans.get(pos.f)->rect());
    }
  }
  
  if (!cellChanged.isEmpty()) {
    changeFrame();
    changeCell(cellChanged);
    Q_EMIT modified();
  }
}

void Timeline::setName(const LayerIdx idx, const std::string_view name) {
  assert(LayerIdx{0} <= idx);
  assert(idx < layerCount());
  layers[+idx].name = name;
  Q_EMIT modified();
}

void Timeline::setDelay(const int newDelay) {
  assert(ctrl_delay.min <= newDelay);
  assert(newDelay <= ctrl_delay.max);
  delay = newDelay;
  Q_EMIT modified();
}

void Timeline::clearSelected() {
  if (locked) return;
  LayerCells nullSpans;
  nullSpans.pushNull(selection.maxF - selection.minF + FrameIdx{1});
  for (LayerIdx l = selection.minL; l <= selection.maxL; ++l) {
    layers[+l].spans.replaceSpan(selection.minF, nullSpans);
    changeSpan(l);
  }
  changeFrame();
  changeCell();
  changePos();
  Q_EMIT modified();
}

void Timeline::copySelected() {
  if (locked) return;
  clipboard.clear();
  for (LayerIdx l = selection.minL; l <= selection.maxL; ++l) {
    const FrameIdx idx = selection.minF;
    const FrameIdx len = selection.maxF - selection.minF + FrameIdx{1};
    clipboard.push_back(layers[+l].spans.extract(idx, len));
  }
}

void Timeline::pasteSelected() {
  if (locked) return;
  if (selection.minL > selection.maxL) return;
  const LayerIdx endLayer = std::min(
    layerCount(), selection.minL + static_cast<LayerIdx>(clipboard.size())
  );
  const FrameIdx frames = frameCount - selection.minF;
  for (LayerIdx l = selection.minL; l < endLayer; ++l) {
    LayerCells spans = clipboard[+(l - selection.minL)].truncateCopy(frames);
    layers[+l].spans.replaceSpan(selection.minF, spans);
    changeSpan(l);
  }
  changeFrame();
  changeCell();
  changePos();
  Q_EMIT modified();
}

void Timeline::lock() {
  assert(!locked);
  locked = true;
}

void Timeline::unlock() {
  assert(locked);
  locked = false;
}

Cell *Timeline::getCell(const CellPos cell) {
  return layers[+cell.l].spans.get(cell.f);
}

Frame Timeline::getFrame(const FrameIdx idx) const {
  Frame frame;
  frame.reserve(layers.size());
  for (const Layer &layer : layers) {
    if (layer.visible) {
      frame.push_back(layer.spans.get(idx));
    }
  }
  return frame;
}

LayerIdx Timeline::layerCount() const {
  return static_cast<LayerIdx>(layers.size());
}

void Timeline::changePos() {
  Q_EMIT posChanged(pos);
  Q_EMIT cellChanged(getCell(pos));
}

void Timeline::changeFrame() {
  Q_EMIT frameChanged(getFrame(pos.f));
}

void Timeline::changeSpan(const LayerIdx idx) {
  Q_EMIT layerChanged(idx, layers[+idx].spans);
}

void Timeline::changeLayers(const LayerIdx begin, const LayerIdx end) {
  assert(begin < end);
  for (LayerIdx l = begin; l != end; ++l) {
    Q_EMIT layerChanged(l, layers[+l].spans);
    Q_EMIT visibilityChanged(l, layers[+l].visible);
    Q_EMIT nameChanged(l, layers[+l].name);
  }
}

void Timeline::changeFrameCount() {
  Q_EMIT frameCountChanged(frameCount);
}

void Timeline::changeLayerCount() {
  Q_EMIT layerCountChanged(layerCount());
}

void Timeline::changeCell(const QRect rect) {
  Q_EMIT cellModified(rect);
}

void Timeline::changeCell() {
  changeCell(toRect(canvasSize));
}

#include "timeline.moc"
