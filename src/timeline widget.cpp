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
  groupName = new GroupNameWidget{this};
  auto *layerScroll = new LayerScrollWidget{this};
  auto *frameScroll = new FrameScrollWidget{this};
  auto *groupScroll = new GroupScrollWidget{this};
  auto *celScroll = new CelScrollWidget{this};
  layers = layerScroll->getChild();
  frames = frameScroll->getChild();
  groups = groupScroll->getChild();
  cels = celScroll->getChild();
  
  CONNECT(layerScroll->verticalScrollBar(), valueChanged, celScroll->verticalScrollBar(), setValue);
  CONNECT(celScroll->verticalScrollBar(), valueChanged, layerScroll->verticalScrollBar(), setValue);
  
  CONNECT(frameScroll->horizontalScrollBar(), valueChanged, celScroll->horizontalScrollBar(), setValue);
  CONNECT(frameScroll->horizontalScrollBar(), valueChanged, groupScroll->horizontalScrollBar(), setValue);
  CONNECT(celScroll->horizontalScrollBar(), valueChanged, frameScroll->horizontalScrollBar(), setValue);
  CONNECT(celScroll->horizontalScrollBar(), valueChanged, groupScroll->horizontalScrollBar(), setValue);
  CONNECT(groupScroll->horizontalScrollBar(), valueChanged, frameScroll->horizontalScrollBar(), setValue);
  CONNECT(groupScroll->horizontalScrollBar(), valueChanged, celScroll->horizontalScrollBar(), setValue);
  
  CONNECT(celScroll, rightMarginChanged, frameScroll, shouldSetRightMargin);
  CONNECT(celScroll, rightMarginChanged, groupScroll, shouldSetRightMargin);
  CONNECT(celScroll, bottomMarginChanged, layerScroll, shouldSetBottomMargin);
  
  CONNECT(layers,    shouldSetVisibility,     this, shouldSetVisibility);
  CONNECT(layers,    shouldIsolateVisibility, this, shouldIsolateVisibility);
  CONNECT(layers,    shouldSetName,           this, shouldSetLayerName);
  
  CONNECT(controls,  shouldNextFrame,         this, shouldNextFrame);
  CONNECT(controls,  shouldInsertLayer,       this, shouldInsertLayer);
  CONNECT(controls,  shouldRemoveLayer,       this, shouldRemoveLayer);
  CONNECT(controls,  shouldMoveLayerUp,       this, shouldMoveLayerUp);
  CONNECT(controls,  shouldMoveLayerDown,     this, shouldMoveLayerDown);
  CONNECT(controls,  shouldExtendCel,         this, shouldExtendCel);
  CONNECT(controls,  shouldSplitCel,          this, shouldSplitCel);
  CONNECT(controls,  shouldSetDelay,          this, shouldSetDelay);
  CONNECT(controls,  shouldToggleAnimation,   this, shouldToggleAnimation);
  
  CONNECT(cels,      shouldBeginSelection,    this, shouldBeginSelection);
  CONNECT(cels,      shouldContinueSelection, this, shouldContinueSelection);
  CONNECT(cels,      shouldEndSelection,      this, shouldEndSelection);
  CONNECT(cels,      shouldClearSelection,    this, shouldClearSelection);
  CONNECT(cels,      shouldSetPos,            this, shouldSetPos);
  
  CONNECT(groups,    shouldSetGroup,          this, shouldSetGroup);
  CONNECT(groups,    shouldMoveGroup,         this, shouldMoveGroup);
  
  CONNECT(groupName, shouldSetName,           this, shouldSetGroupName);
  
  auto *grid = new QGridLayout{this};
  grid->setSpacing(0);
  grid->setContentsMargins(0, 0, 0, 0);
  grid->addWidget(controls,    0, 0);
  grid->addWidget(groupName,   1, 0);
  grid->addWidget(layerScroll, 2, 0);
  grid->addWidget(frameScroll, 0, 1);
  grid->addWidget(groupScroll, 1, 1);
  grid->addWidget(celScroll,   2, 1);
}


void TimelineWidget::setPos(const CelPos pos) {
  cels->setPos(pos);
}

void TimelineWidget::setSelection(const CelRect rect) {
  cels->setSelection(rect);
}

void TimelineWidget::setGroup(const GroupInfo info) {
  groups->setGroup(info);
}

void TimelineWidget::setGroupName(const std::string_view name) {
  groupName->setName(name);
}

void TimelineWidget::setGroupArray(const tcb::span<const Group> array) {
  groups->setGroupArray(array);
}

void TimelineWidget::setVisibility(const LayerIdx layer, const bool visible) {
  layers->setVisibility(layer, visible);
}

void TimelineWidget::setLayerName(const LayerIdx layer, const std::string_view name) {
  layers->setName(layer, name);
}

void TimelineWidget::setLayer(const LayerIdx layer, tcb::span<const CelSpan> spans) {
  cels->setLayer(layer, spans);
}

void TimelineWidget::setFrameCount(const FrameIdx count) {
  frames->setFrameCount(count);
  groups->setFrameCount(count);
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
