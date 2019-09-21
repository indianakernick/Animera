//
//  timeline.cpp
//  Animera
//
//  Created by Indi Kernick on 6/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline.hpp"

#include "cell span.hpp"
#include "composite.hpp"
#include <QtCore/qdir.h>
#include "export png.hpp"
#include "sprite file.hpp"
#include "export pattern.hpp"
#include <Graphics/format.hpp>

namespace {

constexpr CellRect empty_rect = {LayerIdx{0}, FrameIdx{0}, LayerIdx{-1}, FrameIdx{-1}};

}

Timeline::Timeline()
  : currPos{LayerIdx{0}, FrameIdx{0}}, frameCount{0} {}

void Timeline::initDefault() {
  frameCount = FrameIdx{1};
  changeFrameCount();
  Layer layer;
  layer.spans.pushNull(frameCount);
  layer.name = "Layer 0";
  layers.push_back(std::move(layer));
  selection = empty_rect;
  changeLayerCount();
  changeFrame();
  changePos();
  Q_EMIT selectionChanged(selection);
  changeLayers(LayerIdx{0}, LayerIdx{1});
}

void Timeline::optimize() {
  LayerIdx idx{};
  for (Layer &layer : layers) {
    for (CellSpan &span : layer.spans) {
      optimizeCell(*span.cell);
    }
    layer.spans.optimize();
    changeSpan(idx);
    ++idx;
  }
  changeFrame();
  changePos();
}

Error Timeline::serializeHead(QIODevice &dev) const {
  SpriteInfo info;
  info.width = canvasSize.width();
  info.height = canvasSize.height();
  info.layers = layerCount();
  info.frames = frameCount;
  info.delay = 100;
  info.format = canvasFormat;
  return writeAHDR(dev, info);
}

Error Timeline::serializeBody(QIODevice &dev) const {
  for (const Layer &layer : layers) {
    if (Error err = writeLHDR(dev, layer); err) return err;
    for (const CellSpan &span : layer.spans) {
      if (Error err = writeCHDR(dev, span); err) return err;
      if (*span.cell) {
        if (Error err = writeCDAT(dev, span.cell->image, canvasFormat); err) {
          return err;
        }
      }
    }
  }
  return {};
}

Error Timeline::serializeTail(QIODevice &dev) const {
  return writeAEND(dev);
}

Error Timeline::deserializeHead(QIODevice &dev, Format &format, QSize &size) {
  SpriteInfo info;
  if (Error err = readAHDR(dev, info); err) return err;
  canvasSize = size = {info.width, info.height};
  layers.resize(+info.layers);
  frameCount = info.frames;
  canvasFormat = format = info.format;
  return {};
}

Error Timeline::deserializeBody(QIODevice &dev) {
  for (Layer &layer : layers) {
    if (Error err = readLHDR(dev, layer); err) return err;
    for (CellSpan &span : layer.spans) {
      if (Error err = readCHDR(dev, span, canvasFormat); err) return err;
      if (*span.cell) {
        if (Error err = readCDAT(dev, span.cell->image, canvasFormat); err) {
          return err;
        }
      }
    }
  }
  return {};
}

Error Timeline::deserializeTail(QIODevice &dev) {
  if (Error err = readAEND(dev); err) return err;
  selection = empty_rect;
  changeFrameCount();
  changeLayerCount();
  changeFrame();
  changePos();
  Q_EMIT selectionChanged(selection);
  changeLayers(LayerIdx{0}, layerCount());
  return {};
}

CellRect Timeline::selectCells(const ExportOptions &options) const {
  CellRect rect;
  switch (options.layerSelect) {
    case LayerSelect::all_composited:
    case LayerSelect::all:
      rect.minL = LayerIdx{0};
      rect.maxL = layerCount() - LayerIdx{1};
      break;
    case LayerSelect::current:
      rect.minL = rect.maxL = currPos.l;
      break;
    default: Q_UNREACHABLE();
  }
  switch (options.frameSelect) {
    case FrameSelect::all:
      rect.minF = FrameIdx{0};
      rect.maxF = frameCount - FrameIdx{1};
      break;
    case FrameSelect::current:
      rect.minF = rect.maxF = currPos.f;
      break;
    default: Q_UNREACHABLE();
  }
  return rect;
}

namespace {

template <typename Idx>
Idx apply(const Line<Idx> line, const Idx value) {
  return value * line.stride + line.offset;
}

QString getPath(const ExportOptions &options, CellPos pos) {
  QString path = options.directory;
  if (path.back() != QDir::separator()) {
    path.push_back(QDir::separator());
  }
  pos.l = apply(options.layerLine, pos.l);
  pos.f = apply(options.frameLine, pos.f);
  path += evalExportPattern(options.name, pos.l, pos.f);
  path += ".png";
  return path;
}

}

Error Timeline::exportFile(
  const ExportOptions &options,
  const PaletteCSpan palette,
  const QImage &image,
  const CellPos pos
) const {
  return exportPng(getPath(options, pos), palette, image, canvasFormat, options.format);
}

Error Timeline::exportFile(
  const ExportOptions &options,
  const PaletteCSpan palette,
  const Frame &frame,
  const CellPos pos
) const {
  // TODO: don't allocate every time this is called
  QImage result{canvasSize, qimageFormat(canvasFormat)};
  clearImage(result);
  if (canvasFormat == Format::gray) {
    compositeFrame<gfx::YA>(result, palette, frame, canvasFormat);
  } else {
    compositeFrame<gfx::ARGB>(result, palette, frame, canvasFormat);
  }
  return exportFile(options, palette, result, pos);
}

Error Timeline::exportCompRect(
  const ExportOptions &options,
  const PaletteCSpan palette,
  const CellRect rect
) const {
  const LayerIdx rectLayers = rect.maxL - rect.minL + LayerIdx{1};
  Frame frame;
  frame.reserve(+rectLayers);
  std::vector<LayerCells::ConstIterator> iterators;
  iterators.reserve(+rectLayers);
  for (LayerIdx l = rect.minL; l <= rect.maxL; ++l) {
    iterators.push_back(layers[+l].spans.find(rect.minF));
  }
  for (FrameIdx f = rect.minF; f <= rect.maxF; ++f) {
    frame.clear();
    for (LayerIdx l = {}; l != rectLayers; ++l) {
      if (!layers[+(l + rect.minL)].visible) continue;
      if (const Cell *cell = *iterators[+l]; *cell) {
        frame.push_back(*iterators[+l]);
      }
      ++iterators[+l];
    }
    if (Error err = exportFile(options, palette, frame, {rect.minL, f}); err) {
      return err;
    }
  }
  return {};
}

Error Timeline::exportRect(
  const ExportOptions &options,
  const PaletteCSpan palette,
  const CellRect rect
) const {
  for (LayerIdx l = rect.minL; l <= rect.maxL; ++l) {
    const Layer &layer = layers[+l];
    // TODO: does the user want to skip invisible layers?
    if (!layer.visible) continue;
    LayerCells::ConstIterator iter = layer.spans.find(rect.minF);
    for (FrameIdx f = rect.minF; f <= rect.maxF; ++f) {
      if (const Cell *cell = *iter; *cell) {
        if (Error err = exportFile(options, palette, cell->image, {l, f}); err) {
          return err;
        }
      }
      ++iter;
    }
  }
  return {};
}

Error Timeline::exportTimeline(const ExportOptions &options, const PaletteCSpan palette) const {
  const CellRect rect = selectCells(options);
  if (composited(options.layerSelect)) {
    return exportCompRect(options, palette, rect);
  } else {
    return exportRect(options, palette, rect);
  }
}

void Timeline::initCanvas(const Format format, const QSize size) {
  canvasFormat = format;
  canvasSize = size;
}

void Timeline::nextFrame() {
  currPos.f = (currPos.f + FrameIdx{1}) % frameCount;
  changeFrame();
  changePos();
}

void Timeline::prevFrame() {
  currPos.f = (currPos.f - FrameIdx{1} + frameCount) % frameCount;
  changeFrame();
  changePos();
}

void Timeline::layerBelow() {
  currPos.l = std::min(currPos.l + LayerIdx{1}, layerCount() - LayerIdx{1});
  changePos();
}

void Timeline::layerAbove() {
  currPos.l = std::max(currPos.l - LayerIdx{1}, LayerIdx{0});
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
  selection = empty_rect;
  Q_EMIT selectionChanged(selection);
}

void Timeline::insertLayer() {
  Layer layer;
  layer.spans.pushNull(frameCount);
  layer.name = "Layer " + std::to_string(layers.size());
  layers.insert(layers.begin() + +currPos.l, std::move(layer));
  changeLayerCount();
  Q_EMIT selectionChanged(selection);
  changeLayers(currPos.l, layerCount());
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::removeLayer() {
  if (layers.size() == 1) {
    layers.front().spans.clear(frameCount);
    layers.front().name = "Layer 0";
    layers.front().visible = true;
    changeLayers(LayerIdx{0}, LayerIdx{1});
  } else {
    layers.erase(layers.begin() + +currPos.l);
    changeLayerCount();
    Q_EMIT selectionChanged(selection);
    currPos.l = std::min(currPos.l, layerCount() - LayerIdx{1});
    changeLayers(currPos.l, layerCount());
  }
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::moveLayerUp() {
  if (currPos.l == LayerIdx{0}) return;
  std::swap(layers[+(currPos.l - LayerIdx{1})], layers[+currPos.l]);
  changeLayers(currPos.l - LayerIdx{1}, currPos.l + LayerIdx{1});
  changeFrame();
  layerAbove();
  Q_EMIT modified();
}

void Timeline::moveLayerDown() {
  if (currPos.l == layerCount() - LayerIdx{1}) return;
  std::swap(layers[+currPos.l], layers[+(currPos.l + LayerIdx{1})]);
  changeLayers(currPos.l, currPos.l + LayerIdx{2});
  changeFrame();
  layerBelow();
  Q_EMIT modified();
}

void Timeline::insertFrame() {
  ++frameCount;
  changeFrameCount();
  for (LayerIdx l = {}; l != layerCount(); ++l) {
    layers[+l].spans.insertCopy(currPos.f);
    changeSpan(l);
  }
  Q_EMIT selectionChanged(selection);
  nextFrame();
  Q_EMIT modified();
}

void Timeline::insertNullFrame() {
  ++frameCount;
  changeFrameCount();
  for (LayerIdx l = {}; l != layerCount(); ++l) {
    layers[+l].spans.insertNew(currPos.f);
    changeSpan(l);
  }
  Q_EMIT selectionChanged(selection);
  nextFrame();
  Q_EMIT modified();
}

void Timeline::removeFrame() {
  if (frameCount == FrameIdx{1}) {
    for (LayerIdx l = {}; l != layerCount(); ++l) {
      layers[+l].spans.clear(FrameIdx{1});
      changeSpan(l);
    }
  } else {
    --frameCount;
    changeFrameCount();
    for (LayerIdx l = {}; l != layerCount(); ++l) {
      layers[+l].spans.remove(currPos.f);
      changeSpan(l);
    }
    Q_EMIT selectionChanged(selection);
  }
  currPos.f = std::max(currPos.f - FrameIdx{1}, FrameIdx{0});
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::clearCell() {
  layers[+currPos.l].spans.replaceNew(currPos.f);
  changeSpan(currPos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::extendCell() {
  layers[+currPos.l].spans.extend(currPos.f);
  changeSpan(currPos.l);
  nextFrame();
  Q_EMIT modified();
}

void Timeline::splitCell() {
  layers[+currPos.l].spans.split(currPos.f);
  changeSpan(currPos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::growCell(const QRect rect) {
  Cell &cell = *getCell(currPos);
  if (cell) {
    ::growCell(cell, canvasFormat, rect);
    return;
  }
  layers[+currPos.l].spans.replaceNew(currPos.f);
  ::growCell(cell, canvasFormat, rect);
  changeSpan(currPos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::setCurrPos(const CellPos pos) {
  assert(LayerIdx{0} <= pos.l);
  assert(pos.l < layerCount());
  assert(FrameIdx{0} <= pos.f);
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
  assert(LayerIdx{0} <= idx);
  assert(idx < layerCount());
  bool &layerVis = layers[+idx].visible;
  // TODO: Emit signal when layer visibility changed?
  // if (layerVis != visible) {
    layerVis = visible;
  //   Q_EMIT visibilityChanged(idx, visible);
  // }
  changeFrame();
  Q_EMIT modified();
}

void Timeline::setName(const LayerIdx idx, const std::string_view name) {
  assert(LayerIdx{0} <= idx);
  assert(idx < layerCount());
  layers[+idx].name = name;
  // TODO: Emit signal when layer name changed?
  // Q_EMIT nameChanged(idx, name);
  Q_EMIT modified();
}

void Timeline::clearSelected() {
  LayerCells nullSpans;
  nullSpans.pushNull(selection.maxF - selection.minF + FrameIdx{1});
  for (LayerIdx l = selection.minL; l <= selection.maxL; ++l) {
    layers[+l].spans.replaceSpan(selection.minF, nullSpans);
    changeSpan(l);
  }
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::copySelected() {
  clipboard.clear();
  for (LayerIdx l = selection.minL; l <= selection.maxL; ++l) {
    const FrameIdx idx = selection.minF;
    const FrameIdx len = selection.maxF - selection.minF + FrameIdx{1};
    clipboard.push_back(layers[+l].spans.extract(idx, len));
  }
}

void Timeline::pasteSelected() {
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
  changePos();
  Q_EMIT modified();
}

Cell *Timeline::getCell(const CellPos pos) {
  return layers[+pos.l].spans.get(pos.f);
}

Frame Timeline::getFrame(const FrameIdx pos) const {
  Frame frame;
  frame.reserve(layers.size());
  for (const Layer &layer : layers) {
    if (layer.visible) {
      frame.push_back(layer.spans.get(pos));
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

#include "timeline.moc"
