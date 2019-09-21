//
//  timeline controls widget.cpp
//  Animera
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline controls widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "timeline painting.hpp"
#include <QtWidgets/qboxlayout.h>

ControlsWidget::ControlsWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedSize(layer_width, cell_height);
  createWidgets();
  setupLayout();
  connectSignals();
  animTimer.setInterval(ctrl_delay.def);
  animTimer.setTimerType(Qt::PreciseTimer);
}

void ControlsWidget::toggleAnimation() {
  playButton->toggle();
}

void ControlsWidget::toggleTimer() {
  if (animTimer.isActive()) {
    animTimer.stop();
  } else {
    animTimer.start();
  }
}

void ControlsWidget::setInterval(const int interval) {
  animTimer.setInterval(interval);
}

IconPushButtonWidget *ControlsWidget::makePushButton(QPixmap base, const QString &path) {
  paintTimelineButtonIcon(base, path);
  return new IconPushButtonWidget{this, base};
}

IconRadioButtonWidget *ControlsWidget::makeRadioButton(
  QPixmap onPix,
  const QString &onPath,
  const QString &offPath
) {
  QPixmap offPix = onPix;
  paintTimelineButtonIcon(onPix, onPath);
  paintTimelineButtonIcon(offPix, offPath);
  return new IconRadioButtonWidget{this, onPix, offPix};
}

void ControlsWidget::createWidgets() {
  QPixmap baseIcon{cell_width, cell_width};
  paintTimelineButtonBack(baseIcon);
  insertLayerButton = makePushButton(baseIcon, ":/Timeline/add.pbm");
  removeLayerButton = makePushButton(baseIcon, ":/Timeline/remove.pbm");
  moveLayerUpButton = makePushButton(baseIcon, ":/Timeline/move up.pbm");
  moveLayerDownButton = makePushButton(baseIcon, ":/Timeline/move down.pbm");
  extendButton = makePushButton(baseIcon, ":/Timeline/link.pbm");
  splitButton = makePushButton(baseIcon, ":/Timeline/unlink.pbm");
  playButton = makeRadioButton(baseIcon, ":/Timeline/pause.pbm", ":/Timeline/play.pbm");
  delayBox = new NumberInputWidget{this, ctrl_text_rect, ctrl_delay};
  insertLayerButton->setToolTip("Insert Layer");
  removeLayerButton->setToolTip("Remove Layer");
  moveLayerUpButton->setToolTip("Move Layer Up");
  moveLayerDownButton->setToolTip("Move Layer Down");
  extendButton->setToolTip("Extend Linked Cell");
  splitButton->setToolTip("Split Linked Cell");
  playButton->setToolTip("Toggle Playing");
}

void ControlsWidget::setupLayout() {
  auto *layout = new QHBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setAlignment(Qt::AlignLeft);
  layout->addWidget(insertLayerButton);
  layout->addWidget(removeLayerButton);
  layout->addWidget(moveLayerUpButton);
  layout->addWidget(moveLayerDownButton);
  layout->addWidget(extendButton);
  layout->addWidget(splitButton);
  layout->addWidget(playButton);
  layout->addWidget(delayBox);
}

void ControlsWidget::connectSignals() {
  CONNECT(insertLayerButton,   pressed,      this, insertLayer);
  CONNECT(removeLayerButton,   pressed,      this, removeLayer);
  CONNECT(moveLayerUpButton,   pressed,      this, moveLayerUp);
  CONNECT(moveLayerDownButton, pressed,      this, moveLayerDown);
  CONNECT(extendButton,        pressed,      this, extendCell);
  CONNECT(splitButton,         pressed,      this, splitCell);
  CONNECT(playButton,          toggled,      this, toggleTimer);
  CONNECT(animTimer,           timeout,      this, nextFrame);
  CONNECT(delayBox,            valueChanged, this, setInterval);
  // TODO: store delay in Sprite
}

#include "timeline controls widget.moc"
