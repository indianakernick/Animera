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
#include <QtGui/qpainter.h>
#include "timeline widget.hpp"
#include "widget painting.hpp"
#include <QtWidgets/qboxlayout.h>

LayerCellsWidget::LayerCellsWidget(QWidget *parent, TimelineWidget *timeline)
  : QWidget{parent}, timeline{*timeline} {
  setFixedSize(0, cell_height);
  loadIcons();
}

Cell *LayerCellsWidget::appendCell(FrameIdx len) {
  assert(len > 0);
  auto cell = std::make_unique<Cell>(timeline.size, timeline.format, timeline.palette);
  Cell *cellPtr = cell.get();
  frames.push_back({std::move(cell), len});
  addSize(len);
  repaint();
  return cellPtr;
}

void LayerCellsWidget::appendNull(FrameIdx len) {
  assert(len > 0);
  frames.push_back({nullptr, len});
  addSize(len);
  repaint();
}

void LayerCellsWidget::appendFrame() {
  if (frames.empty()) {
    frames.push_back({std::make_unique<Cell>(timeline.size, timeline.format, timeline.palette)});
  } else if (frames.back().cell) {
    CellPtr cell = std::make_unique<Cell>();
    cell->image = frames.back().cell->image;
    frames.push_back({std::move(cell)});
  } else {
    ++frames.back().len;
  }
  addSize(1);
  repaint();
}

Cell *LayerCellsWidget::getCell(FrameIdx frame) {
  for (const LinkedSpan &span : frames) {
    if (frame < span.len) {
      return span.cell.get();
    } else {
      frame -= span.len;
    }
  }
  return nullptr;
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
}

void CellsWidget::changeWidth(const int newWidth) {
  for (LayerCellsWidget *layer : layers) {
    layer->setFixedWidth(newWidth);
  }
}

LayerCellsWidget *CellsWidget::appendLayer() {
  auto *layer = new LayerCellsWidget{this, timeline};
  layout->addWidget(layer);
  layers.push_back(layer);
  return layer;
}

LayerCellsWidget *CellsWidget::getLayer(const LayerIdx layer) {
  assert(layer < layers.size());
  return layers[layer];
}

void CellsWidget::appendFrame() {
  for (LayerCellsWidget *layer : layers) {
    layer->appendFrame();
  }
}

LayerIdx CellsWidget::layerCount() const {
  return static_cast<LayerIdx>(layers.size());
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

void CellsWidget::resizeEvent(QResizeEvent *) {
  Q_EMIT resized();
}

CellScrollWidget::CellScrollWidget(QWidget *parent)
  : ScrollAreaWidget{parent} {
  setFrameShape(NoFrame);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setStyleSheet("background-color:" + glob_main.name());
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

void CellScrollWidget::resizeEvent(QResizeEvent *event) {
  contentResized();
  ScrollAreaWidget::resizeEvent(event);
}

#include "timeline cells widget.moc"
