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

Timeline::Timeline()
  : pos{LayerIdx{0}, FrameIdx{0}}, frameCount{0} {}

void Timeline::initDefault() {
  frameCount = FrameIdx{1};
  Layer layer;
  layer.spans.pushNull(frameCount);
  layer.name = "Layer 0";
  layers.push_back(std::move(layer));
  Group newGroup;
  newGroup.end = frameCount;
  newGroup.name = "Group 0";
  groups.push_back(std::move(newGroup));
  selection = empty_rect;
  group = GroupIdx{0};
  delay = ctrl_delay.def;
  change();
}

void Timeline::optimize() {
  LayerIdx idx{};
  for (Layer &layer : layers) {
    for (CelSpan &span : layer.spans) {
      ::shrinkCel(*span.cel, toRect(canvasSize));
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
  changeCel();
  changePos();
  Q_EMIT selectionChanged(selection);
  changeLayers(LayerIdx{0}, layerCount());
  Q_EMIT delayChanged(delay);
  Q_EMIT groupChanged(getGroup(groups, group));
  Q_EMIT groupNameChanged(groups[+group].name);
  Q_EMIT groupArrayChanged(groups);
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
  layer.spans.begin()->cel->img = std::move(image);
  layer.name = "Layer 0";
  layers.push_back(std::move(layer));
  
  Group newGroup;
  newGroup.end = frameCount;
  newGroup.name = "Group 0";
  groups.push_back(std::move(newGroup));
  
  selection = empty_rect;
  group = GroupIdx{0};
  delay = ctrl_delay.def;
  return {};
}

Error Timeline::importImage(const QString &path) {
  QImage image;
  FileReader reader;
  TRY(reader.open(path));
  TRY(importCelPng(reader.dev(), image, canvasFormat));
  TRY(reader.flush());
  
  Cel *cel = getCel(pos);
  cel->pos = {};
  cel->img = std::move(image);
  
  changeFrame();
  changeCel();
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
  info.groups = static_cast<GroupIdx>(groups.size());
  info.frames = frameCount;
  info.delay = delay;
  info.format = canvasFormat;
  return writeAHDR(dev, info);
}

Error Timeline::serializeBody(QIODevice &dev) const {
  SCOPE_TIME("Timeline::serializeBody");

  TRY(writeGRPS(dev, groups));
  for (const Layer &layer : layers) {
    TRY(writeLHDR(dev, layer));
    for (const CelSpan &span : layer.spans) {
      TRY(writeCHDR(dev, span));
      if (*span.cel) TRY(writeCDAT(dev, span.cel->img, canvasFormat));
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
  groups.resize(+info.groups);
  frameCount = info.frames;
  canvasFormat = format = info.format;
  delay = info.delay;
  return {};
}

Error Timeline::deserializeBody(QIODevice &dev) {
  SCOPE_TIME("Timeline::deserializeBody");

  TRY(readGRPS(dev, groups, frameCount));
  for (Layer &layer : layers) {
    TRY(readLHDR(dev, layer));
    for (CelSpan &span : layer.spans) {
      TRY(readCHDR(dev, span, canvasFormat));
      if (*span.cel) TRY(readCDAT(dev, span.cel->img, canvasFormat));
    }
  }
  return {};
}

Error Timeline::deserializeTail(QIODevice &dev) {
  SCOPE_TIME("Timeline::deserializeTail");

  TRY(readAEND(dev));
  selection = empty_rect;
  group = GroupIdx{0};
  change();
  return {};
}

LayerIdx Timeline::getLayers() const {
  return layerCount();
}

GroupIdx Timeline::getGroups() const {
  return static_cast<GroupIdx>(groups.size());
}

FrameIdx Timeline::getFrames() const {
  return frameCount;
}

CelPos Timeline::getPos() const {
  return pos;
}

CelRect Timeline::getSelection() const {
  return selection;
}

tcb::span<const Layer> Timeline::getLayerArray() const {
  return layers;
}

tcb::span<const Group> Timeline::getGroupArray() const {
  return groups;
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
  changeCel();
  changePos();
  changeGroup(pos.f);
}

void Timeline::prevFrame() {
  if (locked) return;
  pos.f = (pos.f - FrameIdx{1} + frameCount) % frameCount;
  if (frameCount == FrameIdx{1}) return;
  changeFrame();
  changeCel();
  changePos();
  changeGroup(pos.f);
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

CelRect normalize(const CelRect rect) {
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
  const QRect rect = getCel(pos)->rect();
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
  changeCel(rect);
  changePos();
  Q_EMIT modified();
}

void Timeline::moveLayerUp() {
  if (locked) return;
  if (pos.l == LayerIdx{0}) return;
  std::swap(layers[+(pos.l - LayerIdx{1})], layers[+pos.l]);
  changeLayers(pos.l - LayerIdx{1}, pos.l + LayerIdx{1});
  changeFrame();
  const QRect upperRect = getCel({pos.l - LayerIdx{1}, pos.f})->rect();
  const QRect lowerRect = getCel(pos)->rect();
  changeCel(upperRect.united(lowerRect));
  layerAbove();
  Q_EMIT modified();
}

void Timeline::moveLayerDown() {
  if (locked) return;
  if (pos.l == layerCount() - LayerIdx{1}) return;
  std::swap(layers[+pos.l], layers[+(pos.l + LayerIdx{1})]);
  changeLayers(pos.l, pos.l + LayerIdx{2});
  changeFrame();
  const QRect upperRect = getCel(pos)->rect();
  const QRect lowerRect = getCel({pos.l + LayerIdx{1}, pos.f})->rect();
  changeCel(upperRect.united(lowerRect));
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
  insertGroupFrame(groups, pos.f);
  ++pos.f;
  changeGroupArray();
  changeFrame();
  changeCel();
  changePos();
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
    const std::optional<GroupIdx> removed = removeGroupFrame(groups, pos.f);
    if (removed && *removed < group) {
      --group;
      Q_EMIT groupNameChanged(groups[+group].name);
    }
    changeGroupArray();
  }
  pos.f = std::max(pos.f - FrameIdx{1}, FrameIdx{0});
  changeFrame();
  changeCel();
  changePos();
  Q_EMIT modified();
}

void Timeline::clearCel() {
  if (locked) return;
  Layer &layer = layers[+pos.l];
  const QRect rect = layer.spans.get(pos.f)->rect();
  layer.spans.replace(pos.f, false);
  changeSpan(pos.l);
  changeFrame();
  changeCel(rect);
  changePos();
  Q_EMIT modified();
}

void Timeline::extendCel() {
  if (locked) return;
  layers[+pos.l].spans.extend(pos.f);
  changeSpan(pos.l);
  nextFrame();
  Q_EMIT modified();
}

void Timeline::splitCel() {
  if (locked) return;
  layers[+pos.l].spans.split(pos.f);
  changeSpan(pos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::growCel(const QRect rect) {
  Cel &cel = *getCel(pos);
  if (cel) {
    ::growCel(cel, canvasFormat, rect);
    return;
  }
  if (locked) return;
  layers[+pos.l].spans.replace(pos.f, true);
  ::growCel(*getCel(pos), canvasFormat, rect);
  changeSpan(pos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::shrinkCel(const QRect rect) {
  if (locked) return;
  Cel &cel = *getCel(pos);
  if (!cel) return;
  ::shrinkCel(cel, rect);
  if (!cel) {
    layers[+pos.l].spans.optimize();
    changeSpan(pos.l);
    changeFrame();
    changePos();
  }
  Q_EMIT modified();
}

void Timeline::setGroup(const FrameIdx frame) {
  assert(FrameIdx{0} <= frame);
  assert(frame < frameCount);
  if (locked) return;
  pos.f = changeGroup(frame).begin;
  changeFrame();
  changeCel();
  changePos();
}

void Timeline::setGroupName(const std::string_view name) {
  groups[+group].name = name;
  Q_EMIT modified();
}

void Timeline::moveGroup(const GroupIdx idx, const FrameIdx end) {
  if (locked) return;
  if (moveGroupBoundary(groups, idx, end)) {
    Q_EMIT groupArrayChanged(groups);
    if (idx == group || idx + GroupIdx{1} == group) {
      changeGroup(pos.f);
    }
  }
}

void Timeline::splitGroupLeft() {
  if (locked) return;
  if (::splitGroupLeft(groups, pos.f)) {
    ++group;
    changeGroupArray();
  }
}

void Timeline::splitGroupRight() {
  if (locked) return;
  if (::splitGroupRight(groups, pos.f)) {
    ++group;
    changeGroupArray();
    Q_EMIT groupNameChanged(groups[+group].name);
  }
}

void Timeline::mergeGroupLeft() {
  if (locked) return;
  if (::mergeGroupLeft(groups, group)) {
    --group;
    changeGroupArray();
  }
}

void Timeline::mergeGroupRight() {
  if (locked) return;
  if (::mergeGroupRight(groups, group)) {
    changeGroupArray();
  }
}

void Timeline::setPos(const CelPos newPos) {
  assert(LayerIdx{0} <= newPos.l);
  assert(newPos.l < layerCount());
  assert(FrameIdx{0} <= newPos.f);
  assert(newPos.f < frameCount);
  if (locked) return;
  if (pos.f != newPos.f) {
    pos = newPos;
    changeFrame();
    changeCel();
    changePos();
    changeGroup(pos.f);
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
    changeCel(layer.spans.get(pos.f)->rect());
    Q_EMIT modified();
  }
}

void Timeline::isolateVisibility(const LayerIdx idx) {
  assert(LayerIdx{0} <= idx);
  assert(idx < layerCount());
  
  QRect celChanged;
  for (LayerIdx l = {}; l != layerCount(); ++l) {
    Layer &layer = layers[+l];
    if ((l != idx) == layer.visible) {
      layer.visible = !layer.visible;
      Q_EMIT visibilityChanged(l, layer.visible);
      celChanged = celChanged.united(layer.spans.get(pos.f)->rect());
    }
  }
  
  if (!celChanged.isEmpty()) {
    changeFrame();
    changeCel(celChanged);
    Q_EMIT modified();
  }
}

void Timeline::setLayerName(const LayerIdx idx, const std::string_view name) {
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
  LayerCels nullSpans;
  nullSpans.pushNull(selection.maxF - selection.minF + FrameIdx{1});
  for (LayerIdx l = selection.minL; l <= selection.maxL; ++l) {
    layers[+l].spans.replaceSpan(selection.minF, nullSpans);
    changeSpan(l);
  }
  changeFrame();
  changeCel();
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
    LayerCels spans = clipboard[+(l - selection.minL)].truncateCopy(frames);
    layers[+l].spans.replaceSpan(selection.minF, spans);
    changeSpan(l);
  }
  changeFrame();
  changeCel();
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

Cel *Timeline::getCel(const CelPos cel) {
  return layers[+cel.l].spans.get(cel.f);
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
  Q_EMIT celChanged(getCel(pos));
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
    Q_EMIT layerNameChanged(l, layers[+l].name);
  }
}

void Timeline::changeFrameCount() {
  Q_EMIT frameCountChanged(frameCount);
}

void Timeline::changeLayerCount() {
  Q_EMIT layerCountChanged(layerCount());
}

void Timeline::changeCel(const QRect rect) {
  Q_EMIT celModified(rect);
}

void Timeline::changeCel() {
  changeCel(toRect(canvasSize));
}

GroupInfo Timeline::changeGroup(const FrameIdx frame) {
  const GroupInfo info = findGroup(groups, frame);
  group = info.group;
  Q_EMIT groupChanged(info);
  Q_EMIT groupNameChanged(groups[+group].name);
  return info;
}

void Timeline::changeGroupArray() {
  Q_EMIT groupArrayChanged(groups);
  Q_EMIT groupChanged(getGroup(groups, group));
}

#include "timeline.moc"
