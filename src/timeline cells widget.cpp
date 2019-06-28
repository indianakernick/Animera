//
//  timeline cells widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline cells widget.hpp"

#include "serial.hpp"
#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qpainter.h>
#include "timeline widget.hpp"
#include "widget painting.hpp"
#include <QtWidgets/qboxlayout.h>

LayerCellsWidget::LayerCellsWidget(QWidget *parent, TimelineWidget *timeline)
  : QWidget{parent}, timeline{*timeline} {
  setFixedSize(0, cell_height);
  loadIcons();
}

namespace {

using LinkedSpan = LayerCellsWidget::LinkedSpan;

CellPtr copyCell(const CellPtr &cell) {
  return cell ? std::make_unique<Cell>(*cell) : nullptr;
}

// Insert a copy of the previous cell after the index
// Spans are extended instead of copied
void insertCopy(std::vector<LinkedSpan> &frames, const FrameIdx idx) {
  assert(idx >= 0);
  FrameIdx currFrame = 0;
  for (auto f = frames.begin(); f != frames.end(); ++f) {
    LinkedSpan &span = *f;
    currFrame += span.len;
    if (idx < currFrame - 1) {
      ++span.len;
      return;
    } else if (idx == currFrame - 1) {
      if (span.cell) {
        frames.insert(++f, {std::make_unique<Cell>(*span.cell)});
      } else {
        ++span.len;
      }
      return;
    }
  }
  frames.insert(frames.end(), {nullptr});
}

// Insert a new cell after the index
void insertNew(std::vector<LinkedSpan> &frames, const FrameIdx idx, CellPtr cell) {
  assert(idx >= 0);
  FrameIdx currFrame = 0;
  for (auto f = frames.begin(); f != frames.end(); ++f) {
    LinkedSpan &span = *f;
    currFrame += span.len;
    if (idx < currFrame) {
      if (span.cell == cell) {
        ++span.len;
        return;
      } else if (span.len == 1) {
        frames.insert(++f, {std::move(cell)});
        return;
      }
      const FrameIdx rightSize = currFrame - idx;
      const FrameIdx leftSize = span.len - (currFrame - idx);
      if (rightSize == 0) {
        span.len = leftSize;
        frames.insert(++f, {std::move(cell)});
        return;
      } else if (leftSize == 0) {
        span.len = rightSize;
        frames.insert(f, {std::move(cell)});
        return;
      }
      span.len = leftSize;
      f = frames.insert(++f, {std::move(cell)});
      frames.insert(++f, {std::make_unique<Cell>(*span.cell), rightSize});
      return;
    }
  }
  frames.insert(frames.end(), {std::move(cell)});
}

// Replace a cell with a new cell
void replaceNew(std::vector<LinkedSpan> &frames, const FrameIdx idx, CellPtr cell) {
  assert(idx >= 0);
  FrameIdx currFrame = 0;
  for (auto f = frames.begin(); f != frames.end(); ++f) {
    LinkedSpan &span = *f;
    currFrame += span.len;
    if (idx < currFrame) {
      if (span.cell == cell) {
        return;
      }
      if (span.len == 1) {
        span.cell = std::move(cell);
        return;
      }
      const FrameIdx rightSize = currFrame - idx - 1;
      const FrameIdx leftSize = span.len - (currFrame - idx);
      if (rightSize == 0) {
        span.len = leftSize;
        frames.insert(++f, {std::move(cell)});
        return;
      } else if (leftSize == 0) {
        span.len = rightSize;
        frames.insert(f, {std::move(cell)});
        return;
      }
      span.len = leftSize;
      f = frames.insert(++f, {std::move(cell)});
      frames.insert(++f, {copyCell(span.cell), rightSize});
      return;
    }
  }
  assert(false);
}

// Remove a frame
void remove(std::vector<LinkedSpan> &frames, const FrameIdx idx) {
  assert(idx >= 0);
  FrameIdx currFrame = 0;
  for (auto f = frames.begin(); f != frames.end(); ++f) {
    LinkedSpan &span = *f;
    currFrame += span.len;
    if (idx < currFrame) {
      if (--span.len <= 0) {
        frames.erase(f);
      }
      return;
    }
  }
  assert(false);
}

// Get a frame
// Returns null if index is out of range
Cell *get(std::vector<LinkedSpan> &frames, FrameIdx idx) {
  for (const LinkedSpan &span : frames) {
    if (idx < span.len) {
      return span.cell.get();
    } else {
      idx -= span.len;
    }
  }
  return nullptr;
}

}

void LayerCellsWidget::insertFrame(const FrameIdx idx) {
  insertCopy(frames, idx);
  addSize(1);
}

void LayerCellsWidget::insertNullFrame(const FrameIdx idx) {
  // Insert after the idx
  insertNew(frames, idx, nullptr);
  addSize(1);
}

void LayerCellsWidget::removeFrame(const FrameIdx idx) {
  remove(frames, idx);
  addSize(-1);
}

void LayerCellsWidget::clearFrame(const FrameIdx idx) {
  replaceNew(frames, idx, nullptr);
  repaint();
}

void LayerCellsWidget::clearAllFrames(const FrameIdx frameCount) {
  frames.clear();
  frames.push_back({nullptr, frameCount});
  repaint();
}

void LayerCellsWidget::swapWith(LayerCellsWidget &other) {
  std::swap(frames, other.frames);
}

void LayerCellsWidget::cellFromNull(const FrameIdx idx) {
  replaceNew(frames, idx, makeCell());
  repaint();
}

void LayerCellsWidget::appendCell(FrameIdx len) {
  assert(len > 0);
  frames.push_back({makeCell(), len});
  addSize(len);
  repaint();
}

void LayerCellsWidget::appendNull(FrameIdx len) {
  assert(len > 0);
  frames.push_back({nullptr, len});
  addSize(len);
  repaint();
}

void LayerCellsWidget::appendFrame() {
  if (frames.empty()) {
    frames.push_back({makeCell()});
  } else if (frames.back().cell) {
    frames.push_back({std::make_unique<Cell>(frames.back().cell->image)});
  } else {
    ++frames.back().len;
  }
  addSize(1);
  repaint();
}

Cell *LayerCellsWidget::getCell(FrameIdx idx) {
  return get(frames, idx);
}

void LayerCellsWidget::serialize(QIODevice *dev) const {
  assert(dev);
  serializeBytes(dev, static_cast<uint16_t>(frames.size()));
  for (const LinkedSpan &span : frames) {
    serializeBytes(dev, static_cast<uint16_t>(span.len));
    serializeBytes(dev, static_cast<bool>(span.cell));
    if (span.cell) {
      ::serialize(dev, span.cell->image);
    }
  }
}

void LayerCellsWidget::deserialize(QIODevice *dev) {
  assert(dev);
  uint16_t framesSize;
  deserializeBytes(dev, framesSize);
  frames.reserve(framesSize);
  while (framesSize--) {
    uint16_t len;
    deserializeBytes(dev, len);
    bool notNull;
    deserializeBytes(dev, notNull);
    CellPtr cell = nullptr;
    if (notNull) {
      cell = std::make_unique<Cell>();
      ::deserialize(dev, cell->image);
    }
    frames.push_back({std::move(cell), len});
  }
}

void LayerCellsWidget::loadIcons() {
  // @TODO cache
  cellPix = bakeColoredBitmap(":/Timeline/cell.pbm", cell_icon_color);
  beginLinkPix = bakeColoredBitmap(":/Timeline/begin linked cell.pbm", cell_icon_color);
  endLinkPix = bakeColoredBitmap(":/Timeline/end linked cell.pbm", cell_icon_color);
}

void LayerCellsWidget::addSize(const FrameIdx cells) {
  setFixedWidth(width() + cells * cell_icon_step);
}

const Cell *LayerCellsWidget::getLastCell() const {
  return frames.empty() ? nullptr : frames.back().cell.get();
}

CellPtr LayerCellsWidget::makeCell() const {
  return std::make_unique<Cell>(timeline.size, timeline.format, timeline.palette);
}

void LayerCellsWidget::paintBorder(QPainter &painter, const int x) {
  painter.fillRect(
    x - cell_border_offset, 0,
    glob_border_width, cell_height,
    glob_border_color
  );
}

void LayerCellsWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  int x = cell_icon_pad;
  for (const LinkedSpan &span : frames) {
    if (span.cell) {
      if (span.len == 1) {
        painter.drawPixmap(x, cell_icon_pad, cellPix);
        x += cell_icon_step;
        paintBorder(painter, x);
      } else if (span.len > 1) {
        const int between = (span.len - 2) * cell_icon_step;
        painter.drawPixmap(x, cell_icon_pad, beginLinkPix);
        x += cell_icon_step;
        painter.fillRect(
          x - cell_icon_pad - cell_border_offset, cell_icon_pad,
          between + cell_icon_pad + cell_border_offset, cell_icon_size,
          cell_icon_color
        );
        x += between;
        painter.drawPixmap(x, cell_icon_pad, endLinkPix);
        x += cell_icon_step;
        paintBorder(painter, x);
      } else Q_UNREACHABLE();
    } else {
      for (FrameIdx f = 0; f != span.len; ++f) {
        x += cell_icon_step;
        paintBorder(painter, x);
      }
    }
  }
  painter.fillRect(
    0, cell_height - glob_border_width,
    x - cell_icon_pad, glob_border_width,
    glob_border_color
  );
}

CellsWidget::CellsWidget(QWidget *parent, TimelineWidget *timeline)
  : QWidget{parent}, timeline{timeline}, layout{new QVBoxLayout{this}} {
  setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setAlignment(Qt::AlignTop);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  animTimer.setTimerType(Qt::PreciseTimer);
  animTimer.setInterval(100);
  CONNECT(&animTimer, timeout, this, nextFrame);
}

void CellsWidget::nextFrame() {
  pos.f = (pos.f + 1) % frameCount;
  repaint();
  Q_EMIT ensureVisible(getPixelPos());
  Q_EMIT frameChanged(getFrame());
  Q_EMIT posChanged(getCurr(), pos.l, pos.f);
}

void CellsWidget::prevFrame() {
  pos.f = (pos.f - 1 + frameCount) % frameCount;
  repaint();
  Q_EMIT ensureVisible(getPixelPos());
  Q_EMIT frameChanged(getFrame());
  Q_EMIT posChanged(getCurr(), pos.l, pos.f);
}

void CellsWidget::layerBelow() {
  pos.l = std::min(pos.l + 1, layerCount() - 1);
  repaint();
  Q_EMIT ensureVisible(getPixelPos());
  Q_EMIT posChanged(getCurr(), pos.l, pos.f);
}

void CellsWidget::layerAbove() {
  pos.l = std::max(pos.l - 1, 0);
  repaint();
  Q_EMIT ensureVisible(getPixelPos());
  Q_EMIT posChanged(getCurr(), pos.l, pos.f);
}

void CellsWidget::insertLayer(const LayerIdx idx) {
  auto *layer = new LayerCellsWidget{this, timeline};
  layer->appendNull(frameCount);
  layers.insert(layers.begin() + idx, layer);
  layout->insertWidget(idx, layer);
  Q_EMIT frameChanged(getFrame());
  ++pos.l;
  setSize();
}

void CellsWidget::removeLayer(const LayerIdx idx) {
  if (layerCount() == 1) {
    layers[idx]->clearAllFrames(frameCount);
  } else {
    for (size_t l = idx; l < layers.size() - 1; ++l) {
      layers[l]->swapWith(*layers[l + 1]);
    }
    layout->removeWidget(layers.back());
    delete layers.back();
    layers.pop_back();
    --pos.l;
    setSize();
  }
  Q_EMIT frameChanged(getFrame());
}

void CellsWidget::moveLayerUp(const LayerIdx idx) {
  if (idx == 0) return;
  LayerCellsWidget *layer = layers[idx];
  std::swap(layers[idx - 1], layers[idx]);
  layout->removeWidget(layer);
  layout->insertWidget(idx - 1, layer);
  Q_EMIT frameChanged(getFrame());
}

void CellsWidget::moveLayerDown(const LayerIdx idx) {
  if (idx == layerCount() - 1) return;
  LayerCellsWidget *layer = layers[idx];
  std::swap(layers[idx], layers[idx + 1]);
  layout->removeWidget(layer);
  layout->insertWidget(idx + 1, layer);
  Q_EMIT frameChanged(getFrame());
}

void CellsWidget::addFrame() {
  for (LayerCellsWidget *layer : layers) {
    layer->insertFrame(pos.f);
  }
  ++frameCount;
  nextFrame();
  setSize();
}

void CellsWidget::addNullFrame() {
  for (LayerCellsWidget *layer : layers) {
    layer->insertNullFrame(pos.f);
  }
  ++frameCount;
  nextFrame();
  setSize();
}

void CellsWidget::removeFrame() {
  if (frameCount == 1) {
    for (LayerCellsWidget *layer : layers) {
      layer->clearAllFrames(1);
    }
  } else {
    for (LayerCellsWidget *layer : layers) {
      layer->removeFrame(pos.f);
    }
    --frameCount;
    setSize();
  }
  if (pos.f == frameCount) {
    --pos.f;
  }
  --pos.f;
  nextFrame();
}

void CellsWidget::clearFrame() {
  for (LayerCellsWidget *layer : layers) {
    layer->clearFrame(pos.f);
  }
  Q_EMIT frameChanged(getFrame());
  Q_EMIT posChanged(getCurr(), pos.l, pos.f);
}

void CellsWidget::requestCell() {
  layers[pos.l]->cellFromNull(pos.f);
  Q_EMIT frameChanged(getFrame());
  Q_EMIT posChanged(getCurr(), pos.l, pos.f);
}

void CellsWidget::toggleAnimation() {
  if (animTimer.isActive()) {
    animTimer.stop();
  } else {
    animTimer.start();
  }
}

void CellsWidget::setAnimDelay(const int delay) {
  animTimer.setInterval(delay);
}

void CellsWidget::initLayer() {
  frameCount = 1;
  auto *layer = new LayerCellsWidget{this, timeline};
  layers.push_back(layer);
  layout->addWidget(layer);
  Q_EMIT frameChanged({nullptr});
}

void CellsWidget::initCell() {
  layers.back()->appendCell();
  Q_EMIT frameChanged(getFrame());
  Q_EMIT posChanged(getCurr(), pos.l, pos.f);
}

LayerCellsWidget *CellsWidget::appendLayer() {
  auto *layer = new LayerCellsWidget{this, timeline};
  layout->addWidget(layer);
  layers.push_back(layer);
  Q_EMIT frameChanged(getFrame());
  return layer;
}

LayerCellsWidget *CellsWidget::getLayer(const LayerIdx layer) {
  assert(layer < static_cast<int>(layers.size()));
  return layers[layer];
}

void CellsWidget::appendFrame() {
  for (LayerCellsWidget *layer : layers) {
    layer->appendFrame();
  }
  ++frameCount;
}

LayerIdx CellsWidget::layerCount() const {
  return static_cast<LayerIdx>(layers.size());
}

LayerIdx CellsWidget::currLayer() const {
  return pos.l;
}

void CellsWidget::serialize(QIODevice *dev) const {
  serializeBytes(dev, static_cast<uint16_t>(layers.size()));
  for (const LayerCellsWidget *layer : layers) {
    layer->serialize(dev);
  }
}

void CellsWidget::deserialize(QIODevice *dev) {
  uint16_t layersSize;
  deserializeBytes(dev, layersSize);
  for (LayerCellsWidget *layer : layers) {
    delete layer;
  }
  layers.clear();
  layers.reserve(layersSize);
  while (layersSize--) {
    layers.emplace_back(new LayerCellsWidget{this, timeline})->deserialize(dev);
  }
}

Cell *CellsWidget::getCurr() {
  return layers[pos.l]->getCell(pos.f);
}

Frame CellsWidget::getFrame() {
  Frame frame;
  frame.reserve(layers.size());
  for (LayerCellsWidget *layer : layers) {
    frame.push_back(layer->getCell(pos.f));
  }
  return frame;
}

QPoint CellsWidget::getPixelPos() {
  return {pos.f * cell_icon_step, pos.l * cell_height};
}

void CellsWidget::setSize() {
  setFixedSize(frameCount * cell_icon_step, layerCount() * cell_height);
  Q_EMIT resized();
}

void CellsWidget::paintEvent(QPaintEvent *) {
  constexpr int size = 2 * cell_icon_pad + cell_icon_size;
  QPainter painter{this};
  painter.setPen(Qt::NoPen);
  painter.setBrush(cell_curr_color);
  if (layerCount() > 1 || frameCount == 1) {
    painter.drawRect(0, pos.l * cell_height, frameCount * cell_icon_step, size);
  }
  if (frameCount > 1) {
    painter.drawRect(pos.f * cell_icon_step, 0, size, layerCount() * cell_height);
  }
  /*
  if (select.minL <= select.maxL && select.minF <= select.maxF) {
    const int border2 = 2 * glob_border_width;
    const int width = (select.maxF - select.minF) * cell_icon_step + size;
    const int height = (select.maxL - select.minL) * cell_icon_step + size;
    painter.setBrush(cell_select_color);
    painter.drawRect( // top
      select.minF * cell_icon_step, select.minL * cell_height,
      width, glob_border_width
    );
    painter.drawRect( // bottom
      select.minF * cell_icon_step, select.maxL * cell_height + size - glob_border_width,
      width, glob_border_width
    );
    painter.drawRect( // left
      select.minF * cell_icon_step, select.minL * cell_height + glob_border_width,
      glob_border_width, height - border2
    );
    painter.drawRect( // right
      select.maxF * cell_icon_step + size - glob_border_width, select.minL * cell_height + glob_border_width,
      glob_border_width, height - border2
    );
  }
  */
}

void CellsWidget::focusOutEvent(QFocusEvent *) {
  //select = {-1, -1, -1, -1};
}

CellScrollWidget::CellScrollWidget(QWidget *parent)
  : ScrollAreaWidget{parent} {
  setFrameShape(NoFrame);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setStyleSheet("background-color:" + glob_main.name());
  setMinimumSize(cell_icon_step + glob_scroll_width, cell_height + glob_scroll_width);
}

CellsWidget *CellScrollWidget::setChild(CellsWidget *cells) {
  rect = new QWidget{cells};
  rect->setVisible(false);
  CONNECT(cells, ensureVisible, this, ensureVisible);
  CONNECT(cells, resized, this, contentResized);
  setWidget(cells);
  return cells;
}

void CellScrollWidget::contentResized() {
  const QMargins before = viewportMargins();
  adjustMargins();
  const QMargins after = viewportMargins();
  if (before.right() != after.right()) {
    Q_EMIT rightMarginChanged(after.right());
  }
  if (before.bottom() != after.bottom()) {
    Q_EMIT bottomMarginChanged(after.bottom());
  }
}

void CellScrollWidget::ensureVisible(const QPoint pos) {
  // @TODO Why do I have to add 1 here? Bug?
  rect->setGeometry(pos.x(), pos.y(), cell_icon_step + 1, cell_height + 1);
  QScrollArea::ensureWidgetVisible(rect, 0, 0);
}

void CellScrollWidget::resizeEvent(QResizeEvent *event) {
  contentResized();
  ScrollAreaWidget::resizeEvent(event);
}

#include "timeline cells widget.moc"
