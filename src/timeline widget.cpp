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

class LayerWidget final : public QWidget {
public:
  // @TODO a sparse data structure might be better
  struct LinkedSpan {
    CellPtr cell;
    FrameIdx len = 1;
  };

  explicit LayerWidget(TimelineWidget *timeline)
    : QWidget{timeline}, timeline{*timeline} {}

  void appendFrame() {
    frames.push_back({std::make_unique<Cell>(
      timeline.size, timeline.format, timeline.palette
    )});
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
};

TimelineWidget::TimelineWidget(QWidget *parent)
  : QScrollArea{parent} {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFrameShape(NoFrame);
  setStyleSheet("background-color: " + glob_main.name());
}

void TimelineWidget::createInitialCell() {
  layers[0]->appendFrame();
  Q_EMIT frameChanged({layers[0]->getCell(0)});
  Q_EMIT layerVisibility({true});
  Q_EMIT posChange(layers[0]->getCell(0), 0, 0);
}

void TimelineWidget::initialize(const QSize newSize, const Format newFormat) {
  size = newSize;
  format = newFormat;
  layers.emplace_back(new LayerWidget{this});
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
  serializeBytes(&file, static_cast<uint16_t>(layers.size()));
  
  for (const LayerWidget *layer : layers) {
    layer->serialize(&file);
  }
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
  
  uint16_t layersSize;
  deserializeBytes(&file, layersSize);
  for (LayerWidget *layer : layers) {
    delete layer;
  }
  layers.clear();
  layers.reserve(layersSize);
  
  while (layersSize--) {
    layers.emplace_back(new LayerWidget{this})->deserialize(&file);
  }
}

void TimelineWidget::paletteChanged(Palette *newPalette) {
  palette = newPalette;
}

#include "timeline widget.moc"
