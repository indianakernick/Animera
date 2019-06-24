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

class ControlsWidget final : public QWidget {
public:
  explicit ControlsWidget(QWidget *parent)
    : QWidget{parent} {
    setFixedSize(100_px, cell_height);
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
  cells = new CellsWidget{cellScroll, this};
  cellScroll->setWidget(cells);
  
  CONNECT(layerScroll->verticalScrollBar(), valueChanged, cellScroll->verticalScrollBar(), setValue);
  CONNECT(cellScroll->verticalScrollBar(), valueChanged, layerScroll->verticalScrollBar(), setValue);
  
  CONNECT(frameScroll->horizontalScrollBar(), valueChanged, cellScroll->horizontalScrollBar(), setValue);
  CONNECT(cellScroll->horizontalScrollBar(), valueChanged, frameScroll->horizontalScrollBar(), setValue);
  
  CONNECT(frames, widthChanged, cells, changeWidth);
  CONNECT(cells, resized, cellScroll, contentResized);
  CONNECT(cellScroll, rightMarginChanged, frameScroll, changeRightMargin);
  CONNECT(cellScroll, bottomMarginChanged, layerScroll, changeBottomMargin);
  
  CONNECT(cells, posChanged, this, posChanged);
  CONNECT(cells, frameChanged, this, frameChanged);
  
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
  layers->appendLayer(cells->layerCount());
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
    
    layers->appendLayer(cells->layerCount());
    LayerCellsWidget *layer1 = cells->appendLayer();
    layer1->appendNull();
    layer1->appendCell(2);
    layer1->appendCell();
    layer1->appendCell(3);
    layer1->appendNull(5);
    layer1->appendCell();
    
    layers->appendLayer(cells->layerCount());
    LayerCellsWidget *layer2 = cells->appendLayer();
    layer2->appendNull(13);
    
    layers->appendLayer(cells->layerCount());
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
    
    layers->appendLayer(cells->layerCount());
    cells->appendLayer();
    layers->appendLayer(cells->layerCount());
    cells->appendLayer();
    layers->appendLayer(cells->layerCount());
    cells->appendLayer();
    
    cells->appendFrame();
    cells->appendFrame();
    
    for (int f = 0; f != 15; ++f) {
      frames->appendFrame();
    }
    
    for (int f = 0; f != 50; ++f) {
      cells->appendFrame();
      frames->appendFrame();
    }
  }
  
  Q_EMIT frameChanged({cell});
  Q_EMIT posChanged(cell, 0, 0);
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

void TimelineWidget::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
      cells->layerAbove();
      break;
    case Qt::Key_Right:
    case Qt::Key_D:
      cells->nextFrame();
      break;
    case Qt::Key_Down:
    case Qt::Key_S:
      cells->layerBelow();
      break;
    case Qt::Key_Left:
    case Qt::Key_A:
      cells->prevFrame();
      break;
  }
}

#include "timeline widget.moc"
