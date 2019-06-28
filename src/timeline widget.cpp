//
//  timeline widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline widget.hpp"

#include "config.hpp"
#include "serial.hpp"
#include "connect.hpp"
#include <QtGui/qevent.h>
#include <QtCore/qfile.h>
#include <QtWidgets/qgridlayout.h>
#include "timeline cells widget.hpp"
#include "timeline layers widget.hpp"
#include "timeline frames widget.hpp"

// @TODO QSpitter between LayersWidget and CellsWidget?

class ControlsWidget final : public QWidget {
public:
  explicit ControlsWidget(QWidget *parent)
    : QWidget{parent} {
    setFixedSize(101_px, cell_height);
    setStyleSheet("background-color:" + QColor{255, 0, 0}.name());
  }
};

TimelineWidget::TimelineWidget(QWidget *parent)
  : QWidget{parent} {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setStyleSheet("background-color:" + glob_main.name());
  setFocusPolicy(Qt::ClickFocus);
  
  controls = new ControlsWidget{this};
  auto *layerScroll = new LayerScrollWidget{this};
  auto *frameScroll = new FrameScrollWidget{this};
  auto cellScroll = new CellScrollWidget{this};
  layers = layerScroll->setChild(new LayersWidget{layerScroll});
  frames = frameScroll->setChild(new FramesWidget{frameScroll});
  cells = cellScroll->setChild(new CellsWidget{cellScroll, this});
  
  CONNECT(layerScroll->verticalScrollBar(), valueChanged, cellScroll->verticalScrollBar(), setValue);
  CONNECT(cellScroll->verticalScrollBar(), valueChanged, layerScroll->verticalScrollBar(), setValue);
  
  CONNECT(frameScroll->horizontalScrollBar(), valueChanged, cellScroll->horizontalScrollBar(), setValue);
  CONNECT(cellScroll->horizontalScrollBar(), valueChanged, frameScroll->horizontalScrollBar(), setValue);
  
  CONNECT(cells, resized, cellScroll, contentResized);
  CONNECT(cellScroll, rightMarginChanged, frameScroll, changeRightMargin);
  CONNECT(cellScroll, bottomMarginChanged, layerScroll, changeBottomMargin);
  
  CONNECT(cells, posChanged, this, posChanged);
  CONNECT(cells, frameChanged, this, frameChanged);
  CONNECT(layers, visibleChanged, this, visibleChanged);
  
  QGridLayout *grid = new QGridLayout{this};
  setLayout(grid);
  grid->setSpacing(0);
  grid->setContentsMargins(0, 0, 0, 0);
  grid->addWidget(controls, 0, 0);
  grid->addWidget(layerScroll, 1, 0);
  grid->addWidget(frameScroll, 0, 1);
  grid->addWidget(cellScroll, 1, 1);
}

void TimelineWidget::initCanvas(const QSize newSize, const Format newFormat) {
  size = newSize;
  format = newFormat;
  frames->addFrame();
  cells->initLayer();
  layers->insertLayer(0);
  cells->initCell();
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

void TimelineWidget::changePalette(Palette *newPalette) {
  palette = newPalette;
}

void TimelineWidget::addLayer() {
  const LayerIdx layer = cells->currLayer();
  cells->insertLayer(layer);
  layers->insertLayer(layer);
  cells->layerAbove();
}

void TimelineWidget::removeLayer() {
  const LayerIdx layer = cells->currLayer();
  cells->removeLayer(layer);
  layers->removeLayer(layer);
  cells->layerBelow();
}

void TimelineWidget::moveLayerUp() {
  const LayerIdx layer = cells->currLayer();
  cells->moveLayerUp(layer);
  layers->moveLayerUp(layer);
  cells->layerAbove();
}

void TimelineWidget::moveLayerDown() {
  const LayerIdx layer = cells->currLayer();
  cells->moveLayerDown(layer);
  layers->moveLayerDown(layer);
  cells->layerBelow();
}

void TimelineWidget::toggleLayerVisible() {
  layers->toggleVisible(cells->currLayer());
}

void TimelineWidget::addFrame() {
  cells->addFrame();
  frames->addFrame();
}

void TimelineWidget::addNullFrame() {
  cells->addNullFrame();
  frames->addFrame();
}

void TimelineWidget::removeFrame() {
  cells->removeFrame();
  frames->removeFrame();
}

void TimelineWidget::clearFrame() {
  cells->clearFrame();
}

void TimelineWidget::requestCell() {
  cells->requestCell();
}

void TimelineWidget::toggleAnimation() {
  cells->toggleAnimation();
}

void TimelineWidget::layerAbove() {
  cells->layerAbove();
}

void TimelineWidget::layerBelow() {
  cells->layerBelow();
}

void TimelineWidget::nextFrame() {
  cells->nextFrame();
}

void TimelineWidget::prevFrame() {
  cells->prevFrame();
}

#include "timeline widget.moc"
