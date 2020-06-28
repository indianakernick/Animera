//
//  timeline controls widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 7/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "timeline controls widget.hpp"

#include "connect.hpp"
#include "config geometry.hpp"
#include "timeline painting.hpp"
#include <QtWidgets/qboxlayout.h>

ControlsWidget::ControlsWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedSize(layer_width, cel_height);
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
  Q_EMIT shouldSetDelay(delay);
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
  QPixmap baseIcon{cel_width, cel_width};
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
  extendButton->setToolTip("Extend Linked Cel");
  splitButton->setToolTip("Split Linked Cel");
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
  CONNECT(insertLayerButton,   pressed,      this, shouldInsertLayer);
  CONNECT(removeLayerButton,   pressed,      this, shouldRemoveLayer);
  CONNECT(moveLayerUpButton,   pressed,      this, shouldMoveLayerUp);
  CONNECT(moveLayerDownButton, pressed,      this, shouldMoveLayerDown);
  CONNECT(extendButton,        pressed,      this, shouldExtendCel);
  CONNECT(splitButton,         pressed,      this, shouldSplitCel);
  CONNECT(playButton,          toggled,      this, toggleTimer);
  CONNECT(playButton,          toggled,      this, shouldToggleAnimation);
  CONNECT(animTimer,           timeout,      this, shouldNextFrame);
  CONNECT(delayBox,            valueChanged, this, changeDelay);
}

#include "timeline controls widget.moc"
