//
//  timeline widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 10/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "timeline widget.hpp"

#include "connect.hpp"
#include <QtWidgets/qgridlayout.h>
#include "timeline cels widget.hpp"
#include "timeline layers widget.hpp"
#include "timeline frames widget.hpp"
#include "timeline controls widget.hpp"

TimelineWidget::TimelineWidget(QWidget *parent)
  : QWidget{parent} {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  
  controls = new ControlsWidget{this};
  auto *layerScroll = new LayerScrollWidget{this};
  auto *frameScroll = new FrameScrollWidget{this};
  auto *celScroll = new CelScrollWidget{this};
  layers = layerScroll->getChild();
  frames = frameScroll->getChild();
  cels = celScroll->getChild();
  
  CONNECT(layerScroll->verticalScrollBar(), valueChanged, celScroll->verticalScrollBar(), setValue);
  CONNECT(celScroll->verticalScrollBar(), valueChanged, layerScroll->verticalScrollBar(), setValue);
  
  CONNECT(frameScroll->horizontalScrollBar(), valueChanged, celScroll->horizontalScrollBar(), setValue);
  CONNECT(celScroll->horizontalScrollBar(), valueChanged, frameScroll->horizontalScrollBar(), setValue);
  
  CONNECT(celScroll, rightMarginChanged, frameScroll, shouldSetRightMargin);
  CONNECT(celScroll, bottomMarginChanged, layerScroll, shouldSetBottomMargin);
  
  CONNECT(layers,   shouldSetVisibility,     this, shouldSetVisibility);
  CONNECT(layers,   shouldSetName,           this, shouldSetName);
  
  CONNECT(controls, shouldNextFrame,         this, shouldNextFrame);
  CONNECT(controls, shouldInsertLayer,       this, shouldInsertLayer);
  CONNECT(controls, shouldRemoveLayer,       this, shouldRemoveLayer);
  CONNECT(controls, shouldMoveLayerUp,       this, shouldMoveLayerUp);
  CONNECT(controls, shouldMoveLayerDown,     this, shouldMoveLayerDown);
  CONNECT(controls, shouldExtendCel,         this, shouldExtendCel);
  CONNECT(controls, shouldSplitCel,          this, shouldSplitCel);
  CONNECT(controls, shouldSetDelay,          this, shouldSetDelay);
  CONNECT(controls, shouldToggleAnimation,   this, shouldToggleAnimation);
  
  CONNECT(cels,     shouldBeginSelection,    this, shouldBeginSelection);
  CONNECT(cels,     shouldContinueSelection, this, shouldContinueSelection);
  CONNECT(cels,     shouldEndSelection,      this, shouldEndSelection);
  CONNECT(cels,     shouldClearSelection,    this, shouldClearSelection);
  CONNECT(cels,     shouldSetPos,            this, shouldSetPos);
  
  auto *grid = new QGridLayout{this};
  grid->setSpacing(0);
  grid->setContentsMargins(0, 0, 0, 0);
  grid->addWidget(controls, 0, 0);
  grid->addWidget(layerScroll, 1, 0);
  grid->addWidget(frameScroll, 0, 1);
  grid->addWidget(celScroll, 1, 1);
}


void TimelineWidget::setPos(const CelPos pos) {
  cels->setPos(pos);
}

void TimelineWidget::setSelection(const CelRect rect) {
  cels->setSelection(rect);
}

void TimelineWidget::setVisibility(const LayerIdx layer, const bool visible) {
  layers->setVisibility(layer, visible);
}

void TimelineWidget::setName(const LayerIdx layer, const std::string_view name) {
  layers->setName(layer, name);
}

void TimelineWidget::setLayer(const LayerIdx layer, std::span<const CelSpan> spans) {
  cels->setLayer(layer, spans);
}

void TimelineWidget::setFrameCount(const FrameIdx count) {
  frames->setFrameCount(count);
  cels->setFrameCount(count);
}

void TimelineWidget::setLayerCount(const LayerIdx count) {
  layers->setLayerCount(count);
  cels->setLayerCount(count);
}

void TimelineWidget::toggleAnimation() {
  controls->toggleAnimation();
}

void TimelineWidget::setDelay(const int delay) {
  controls->setDelay(delay);
}

#include "timeline widget.moc"
