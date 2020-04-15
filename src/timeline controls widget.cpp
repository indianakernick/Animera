//
//  timeline controls widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 7/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
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
  animTimer.setTimerType(Qt::PreciseTimer);
}

void ControlsWidget::toggleAnimation() {
  playButton->toggle();
}

void ControlsWidget::setDelay(const int delay) {
  animTimer.setInterval(delay);
  delayBox->setValue(delay);
}

void ControlsWidget::toggleTimer() {
  if (animTimer.isActive()) {
    animTimer.stop();
  } else {
    animTimer.start();
  }
}

void ControlsWidget::changeDelay(const int delay) {
  animTimer.setInterval(delay);
  Q_EMIT delayChanged(delay);
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
  insertLayerButton = makePushButton(baseIcon, ":/Timeline/add.png");
  removeLayerButton = makePushButton(baseIcon, ":/Timeline/remove.png");
  moveLayerUpButton = makePushButton(baseIcon, ":/Timeline/move up.png");
  moveLayerDownButton = makePushButton(baseIcon, ":/Timeline/move down.png");
  extendButton = makePushButton(baseIcon, ":/Timeline/link.png");
  splitButton = makePushButton(baseIcon, ":/Timeline/unlink.png");
  playButton = makeRadioButton(baseIcon, ":/Timeline/pause.png", ":/Timeline/play.png");
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
  CONNECT(playButton,          toggled,      this, animationToggled);
  CONNECT(animTimer,           timeout,      this, nextFrame);
  CONNECT(delayBox,            valueChanged, this, changeDelay);
}

#include "timeline controls widget.moc"
