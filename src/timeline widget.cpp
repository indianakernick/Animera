//
//  timeline widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline widget.hpp"

#include "cell.hpp"
#include "serial.hpp"
#include "config.hpp"
#include <QtCore/qfile.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qscrollarea.h>

class LayerCellsWidget final : public QWidget {
public:
  // @TODO a sparse data structure might be better
  struct LinkedSpan {
    CellPtr cell;
    FrameIdx len = 1;
  };

  LayerCellsWidget(QWidget *parent, TimelineWidget *timeline)
    : QWidget{parent}, timeline{*timeline} {
    setFixedSize(200_px, 10_px);
  }

  void appendFrame() {
    frames.push_back({std::make_unique<Cell>(
      timeline.size, timeline.format, timeline.palette
    )});
    repaint();
  }
  
  Cell *getCell(FrameIdx frame) {
    for (const LinkedSpan &span : frames) {
      if (frame < span.len) {
        return span.cell.get();
      } else {
        frame -= span.len;
      }
    }
    return nullptr;
  }
  
  void serialize(QIODevice *dev) const {
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
  void deserialize(QIODevice *dev) {
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

private:
  TimelineWidget &timeline;
  std::vector<LinkedSpan> frames;
  
  const Cell *getLastCell() const {
    return frames.empty() ? nullptr : frames.back().cell.get();
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    int x = 0;
    for (const LinkedSpan &span : frames) {
      if (span.cell) {
        if (span.len == 1) {
          QRegion cellRegion{QBitmap{":/Timeline/cell.pbm"}.scaled(8_px, 8_px)};
          cellRegion.translate(x, 1_px);
          painter.setClipRegion(cellRegion);
          painter.fillRect(x, 1_px, 8_px, 8_px, QColor{0, 0, 0});
        }
      }
      x += span.len * 10_px;
    }
  }
};

class ControlsWidget final : public QWidget {
public:
  explicit ControlsWidget(QWidget *parent)
    : QWidget{parent} {
    setFixedSize(100_px, 12_px);
    setStyleSheet("background-color:" + QColor{255, 0, 0}.name());
  }
};

class LayersWidget final : public QWidget {
public:
  explicit LayersWidget(QWidget *parent)
    : QWidget{parent} {}
};

class FramesWidget final : public QWidget {
public:
  explicit FramesWidget(QWidget *parent)
    : QWidget{parent} {}
};

class CellsWidget final : public QWidget {
public:
  CellsWidget(QWidget *parent, TimelineWidget *timeline)
    : QWidget{parent}, timeline{timeline}, boxLayout{new QVBoxLayout{this}} {
    setLayout(boxLayout);
  }
  
  void appendLayer() {
    auto *layer = new LayerCellsWidget{this, timeline};
    boxLayout->addWidget(layer);
    layers.push_back(layer);
  }
  
  LayerCellsWidget *getLayer(const LayerIdx layer) {
    assert(layer < layers.size());
    return layers[layer];
  }
  
  void serialize(QIODevice *dev) {
    serializeBytes(dev, static_cast<uint16_t>(layers.size()));
    for (const LayerCellsWidget *layer : layers) {
      layer->serialize(dev);
    }
  }
  void deserialize(QIODevice *dev) {
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
  
private:
  TimelineWidget *timeline;
  QVBoxLayout *boxLayout;
  std::vector<LayerCellsWidget *> layers;
};

class LayerScrollWidget final : public QScrollArea {
public:
  explicit LayerScrollWidget(QWidget *parent)
    : QScrollArea{parent} {
    setFrameShape(NoFrame);
    setFixedWidth(100_px);
    setStyleSheet("background-color:" + QColor{0, 255, 0}.name());
  }
};

class FrameScrollWidget final : public QScrollArea {
public:
  explicit FrameScrollWidget(QWidget *parent)
    : QScrollArea{parent} {
    setFrameShape(NoFrame);
    setFixedHeight(12_px);
    setStyleSheet("background-color:" + QColor{0, 0, 255}.name());
  }
};

class CellScrollWidget final : public QScrollArea {
public:
  explicit CellScrollWidget(QWidget *parent)
    : QScrollArea{parent} {
    setFrameShape(NoFrame);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background-color:" + QColor{255, 255, 0}.name());
  }
};

TimelineWidget::TimelineWidget(QWidget *parent)
  : QWidget{parent} {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setStyleSheet("background-color:" + glob_main.name());
  
  controls = new ControlsWidget{this};
  auto *layerScroll = new LayerScrollWidget{this};
  auto *frameScroll = new FrameScrollWidget{this};
  auto cellScroll = new CellScrollWidget{this};
  layers = new LayersWidget{layerScroll};
  layerScroll->setWidget(layers);
  frames = new FramesWidget{frameScroll};
  frameScroll->setWidget(frames);
  cells = new CellsWidget{cellScroll, this};
  cellScroll->setWidget(cells);
  
  QGridLayout *grid = new QGridLayout{this};
  setLayout(grid);
  grid->setSpacing(0);
  grid->setContentsMargins(0, 0, 0, 0);
  grid->addWidget(controls, 0, 0);
  grid->addWidget(layerScroll, 1, 0);
  grid->addWidget(frameScroll, 0, 1);
  grid->addWidget(cellScroll, 1, 1);
}

void TimelineWidget::createInitialCell() {
  cells->appendLayer();
  cells->getLayer(0)->appendFrame();
  Q_EMIT frameChanged({cells->getLayer(0)->getCell(0)});
  Q_EMIT layerVisibility({true});
  Q_EMIT posChange(cells->getLayer(0)->getCell(0), 0, 0);
}

void TimelineWidget::initialize(const QSize newSize, const Format newFormat) {
  size = newSize;
  format = newFormat;
}

namespace {

// @TODO maybe this could be improved?
// Look at PNG
constexpr char const magic_number[] = {'P', 'I', 'X', '2'};

}

void TimelineWidget::save(const QString &path) const {
  QFile file{path};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    // @TODO handle this properly
    throw std::exception{};
  }
  file.write(magic_number, sizeof(magic_number));
  serializeBytes(&file, format);
  if (format == Format::palette) {
    serialize(&file, *palette);
  }
  
  serializeBytes(&file, static_cast<uint16_t>(size.width()));
  serializeBytes(&file, static_cast<uint16_t>(size.height()));
  cells->serialize(&file);
}

void TimelineWidget::load(const QString &path) {
  // @TODO handle corrupted or invalid files properly
  QFile file{path};
  if (!file.open(QIODevice::ReadOnly)) {
    throw std::exception{};
  }
  char header[sizeof(magic_number)];
  file.read(header, sizeof(magic_number));
  if (std::memcmp(header, magic_number, sizeof(magic_number)) != 0) {
    throw std::exception{};
  }
  deserializeBytes(&file, format);
  if (format == Format::palette) {
    deserialize(&file, *palette);
  }
  
  uint16_t width;
  uint16_t height;
  deserializeBytes(&file, width);
  deserializeBytes(&file, height);
  size = {width, height};
  cells->deserialize(&file);
}

void TimelineWidget::paletteChanged(Palette *newPalette) {
  palette = newPalette;
}

#include "timeline widget.moc"
