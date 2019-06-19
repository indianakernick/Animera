//
//  timeline widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline widget.hpp"

#include "serial.hpp"
#include "config.hpp"
#include "cell impls.hpp"
#include <QtCore/qfile.h>

class LayerWidget final : public QWidget {
public:
  explicit LayerWidget(TimelineWidget *timeline)
    : QWidget{timeline}, timeline{*timeline} {}

  void appendSource() {
    frames.push_back(std::make_unique<SourceCell>(
      timeline.size, timeline.format, timeline.palette
    ));
  }
  void appendDuplicate() {
    frames.push_back(std::make_unique<DuplicateCell>(getLastCell()));
  }
  void appendTransform() {
    frames.push_back(std::make_unique<TransformCell>(getLastCell()));
  }
  
  Cell *getCell(FrameIdx frame) {
    return frame < frames.size() ? frames[frame].get() : nullptr;
  }
  
  void serialize(QIODevice *dev) const {
    assert(dev);
    ::serialize(dev, static_cast<uint16_t>(frames.size()));
    for (const CellPtr &cell : frames) {
      serializeCell(dev, cell.get());
    }
  }
  void deserialize(QIODevice *dev) {
    assert(dev);
    uint16_t framesSize;
    ::deserialize(dev, framesSize);
    frames.reserve(framesSize);
    while (framesSize--) {
      frames.push_back(deserializeCell(dev));
    }
  }

private:
  TimelineWidget &timeline;
  Frames frames;
  
  const Cell *getLastCell() const {
    return frames.empty() ? nullptr : frames.back().get();
  }
};

TimelineWidget::TimelineWidget(QWidget *parent)
  : QScrollArea{parent} {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFrameShape(NoFrame);
  setStyleSheet("background-color: " + glob_main.name());
}

void TimelineWidget::createInitialCell() {
  layers[0]->appendSource();
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
  serialize(&file, format);
  if (format == Format::palette) {
    serialize(&file, *palette);
  }
  
  serialize(&file, static_cast<uint16_t>(size.width()));
  serialize(&file, static_cast<uint16_t>(size.height()));
  serialize(&file, static_cast<uint16_t>(layers.size()));
  
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
  deserialize(&file, format);
  if (format == Format::palette) {
    deserialize(&file, *palette);
  }
  
  uint16_t width;
  uint16_t height;
  deserialize(&file, width);
  deserialize(&file, height);
  size = {width, height};
  
  uint16_t layersSize;
  deserialize(&file, layersSize);
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
