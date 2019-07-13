//
//  timeline widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline widget.hpp"

#include "connect.hpp"
#include <QtWidgets/qgridlayout.h>
#include "timeline cells widget.hpp"
#include "timeline layers widget.hpp"
#include "timeline frames widget.hpp"
#include "timeline controls widget.hpp"

// @TODO QSpitter between LayersWidget and CellsWidget?

TimelineWidget::TimelineWidget(QWidget *parent)
  : QWidget{parent} {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  
  controls = new ControlsWidget{this};
  auto *layerScroll = new LayerScrollWidget{this};
  auto *frameScroll = new FrameScrollWidget{this};
  auto cellScroll = new CellScrollWidget{this};
  layers = layerScroll->getChild();
  frames = frameScroll->getChild();
  cells = cellScroll->getChild();
  
  CONNECT(layerScroll->verticalScrollBar(), valueChanged, cellScroll->verticalScrollBar(), setValue);
  CONNECT(cellScroll->verticalScrollBar(), valueChanged, layerScroll->verticalScrollBar(), setValue);
  
  CONNECT(frameScroll->horizontalScrollBar(), valueChanged, cellScroll->horizontalScrollBar(), setValue);
  CONNECT(cellScroll->horizontalScrollBar(), valueChanged, frameScroll->horizontalScrollBar(), setValue);
  
  CONNECT(cellScroll, rightMarginChanged, frameScroll, shouldSetRightMargin);
  CONNECT(cellScroll, bottomMarginChanged, layerScroll, shouldSetBottomMargin);
  
  CONNECT(layers, visibilityChanged, this, visibilityChanged);
  CONNECT(layers, nameChanged, this, nameChanged);
  CONNECT(controls, nextFrame, this, nextFrame);
  
  QGridLayout *grid = new QGridLayout{this};
  setLayout(grid);
  grid->setSpacing(0);
  grid->setContentsMargins(0, 0, 0, 0);
  grid->addWidget(controls, 0, 0);
  grid->addWidget(layerScroll, 1, 0);
  grid->addWidget(frameScroll, 0, 1);
  grid->addWidget(cellScroll, 1, 1);
}


void TimelineWidget::setCurrPos(const CellPos pos) {
  cells->setCurrPos(pos);
}

void TimelineWidget::setVisibility(const LayerIdx layer, const bool visible) {
  layers->setVisibility(layer, visible);
}

void TimelineWidget::setName(const LayerIdx layer, const std::string_view name) {
  layers->setName(layer, name);
}

void TimelineWidget::setLayer(const LayerIdx layer, const Spans &spans) {
  cells->setLayer(layer, spans);
}

void TimelineWidget::setFrameCount(const FrameIdx count) {
  frames->setFrameCount(count);
  cells->setFrameCount(count);
}

void TimelineWidget::setLayerCount(const LayerIdx count) {
  layers->setLayerCount(count);
  cells->setLayerCount(count);
}

void TimelineWidget::toggleAnimation() {
  controls->toggleAnimation();
}

#include "timeline widget.moc"
