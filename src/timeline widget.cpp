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
#include "widget painting.hpp"
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qscrollarea.h>

inline const QColor cell_icon_color = glob_light_2;
constexpr int cell_icon_pad = 1_px;
constexpr int cell_icon_size = 8_px;
constexpr int cell_icon_step = 2 * cell_icon_pad + cell_icon_size + glob_border_width;
constexpr int layer_height = cell_icon_step;
constexpr int cell_border_offset = cell_icon_pad + glob_border_width;

class LayerCellsWidget final : public QWidget {
public:
  // @TODO a sparse data structure might be better
  struct LinkedSpan {
    // Does this need to be a std::unique_ptr?
    CellPtr cell;
    FrameIdx len = 1;
  };

  LayerCellsWidget(QWidget *parent, TimelineWidget *timeline)
    : QWidget{parent}, timeline{*timeline} {
    setFixedSize(0, layer_height);
    loadIcons();
  }

  Cell *appendCell(FrameIdx len = 1) {
    assert(len > 0);
    auto cell = std::make_unique<Cell>(timeline.size, timeline.format, timeline.palette);
    Cell *cellPtr = cell.get();
    frames.push_back({std::move(cell), len});
    addSize(len);
    repaint();
    return cellPtr;
  }
  
  void appendNull(FrameIdx len = 1) {
    assert(len > 0);
    frames.push_back({nullptr, len});
    addSize(len);
    repaint();
  }
  
  void appendFrame() {
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
  QPixmap cellPix;
  QPixmap beginLinkPix;
  QPixmap endLinkPix;
  
  void loadIcons() {
    cellPix = bakeColoredBitmap(":/Timeline/cell.pbm", cell_icon_color);
    beginLinkPix = bakeColoredBitmap(":/Timeline/begin linked cell.pbm", cell_icon_color);
    endLinkPix = bakeColoredBitmap(":/Timeline/end linked cell.pbm", cell_icon_color);
  }
  
  void addSize(const FrameIdx cells) {
    setFixedWidth(width() + cells * cell_icon_step);
  }
  
  const Cell *getLastCell() const {
    return frames.empty() ? nullptr : frames.back().cell.get();
  }
  
  void paintBorder(QPainter &painter, const int x) {
    painter.fillRect(
      x - cell_border_offset, 0,
      glob_border_width, layer_height,
      glob_border_color
    );
  }
  
  void paintEvent(QPaintEvent *) override {
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
      0, layer_height - glob_border_width,
      x - cell_icon_pad, glob_border_width,
      glob_border_color
    );
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
    boxLayout->setSpacing(0);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->setAlignment(Qt::AlignTop);
    boxLayout->setSizeConstraint(QLayout::SetFixedSize);
  }
  
  LayerCellsWidget *appendLayer() {
    auto *layer = new LayerCellsWidget{this, timeline};
    boxLayout->addWidget(layer);
    layers.push_back(layer);
    return layer;
  }
  
  LayerCellsWidget *getLayer(const LayerIdx layer) {
    assert(layer < layers.size());
    return layers[layer];
  }
  
  void appendFrame() {
    for (LayerCellsWidget *layer : layers) {
      layer->appendFrame();
    }
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
    setStyleSheet("background-color:" + glob_main.name());
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
  LayerCellsWidget *layer = cells->appendLayer();
  assert(layer);
  Cell *cell = layer->appendCell();
  assert(cell);
  
  {
    // @TODO remove
    layer->appendCell();
    layer->appendCell();
    layer->appendNull(2);
    layer->appendCell(8);
    
    LayerCellsWidget *layer1 = cells->appendLayer();
    layer1->appendNull();
    layer1->appendCell(2);
    layer1->appendCell();
    layer1->appendCell(3);
    layer1->appendNull(5);
    layer1->appendCell();
    
    LayerCellsWidget *layer2 = cells->appendLayer();
    layer2->appendNull(13);
    
    LayerCellsWidget *layer3 = cells->appendLayer();
    layer3->appendCell();
    layer3->appendCell();
    layer3->appendCell();
    layer3->appendCell();
    layer3->appendCell();
    layer3->appendCell();
    layer3->appendCell();
    layer3->appendCell();
    layer3->appendNull(5);
    
    cells->appendFrame();
    cells->appendFrame();
  }
  
  Q_EMIT frameChanged({cell});
  Q_EMIT layerVisibility({true});
  Q_EMIT posChange(cell, 0, 0);
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
