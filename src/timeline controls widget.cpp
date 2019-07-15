//
//  timeline controls widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline controls widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qboxlayout.h>

// @TODO remove layer button appears when hoving layer name
// would this be more intuitive than a button that removes the current layer?

ControlsWidget::ControlsWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedSize(layer_width, cell_height);
  createWidgets();
  setupLayout();
  connectSignals();
  animTimer.setInterval(100);
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

// @TODO this is duplicated in LayerNameWidget
void ControlsWidget::paintBack(QPixmap &pixmap) {
  QPainter painter{&pixmap};
  painter.fillRect(
    0, 0,
    cell_width - glob_border_width, cell_width - glob_border_width,
    glob_main
  );
  painter.fillRect(
    cell_width - glob_border_width, 0,
    glob_border_width, cell_width,
    glob_border_color
  );
  painter.fillRect(
    0, cell_width - glob_border_width,
    cell_width - glob_border_width, glob_border_width,
    glob_border_color
  );
}

void ControlsWidget::paintIcon(QPixmap &pixmap, const QString &path) {
  QPainter painter{&pixmap};
  QBitmap bitmap{path};
  bitmap = bitmap.scaled(bitmap.size() * glob_scale);
  QRegion region{bitmap};
  region.translate(cell_icon_pad, cell_icon_pad);
  painter.setClipRegion(region);
  painter.fillRect(
    cell_icon_pad, cell_icon_pad,
    cell_icon_size, cell_icon_size,
    cell_icon_color
  );
}

IconPushButtonWidget *ControlsWidget::makePushButton(QPixmap base, const QString &path) {
  paintIcon(base, path);
  return new IconPushButtonWidget{this, base};
}

IconRadioButtonWidget *ControlsWidget::makeRadioButton(
  QPixmap onPix,
  const QString &onPath,
  const QString &offPath
) {
  QPixmap offPix = onPix;
  paintIcon(onPix, onPath);
  paintIcon(offPix, offPath);
  return new IconRadioButtonWidget{this, onPix, offPix};
}

void ControlsWidget::createWidgets() {
  QPixmap baseIcon{cell_width, cell_width};
  paintBack(baseIcon);
  insertLayerButton = makePushButton(baseIcon, ":/Timeline/add.pbm");
  removeLayerButton = makePushButton(baseIcon, ":/Timeline/remove.pbm");
  moveLayerUpButton = makePushButton(baseIcon, ":/Timeline/move up.pbm");
  moveLayerDownButton = makePushButton(baseIcon, ":/Timeline/move down.pbm");
  playButton = makeRadioButton(baseIcon, ":/Timeline/pause.pbm", ":/Timeline/play.pbm");
  insertLayerButton->setToolTip("Insert Layer");
  removeLayerButton->setToolTip("Remove Layer");
  moveLayerUpButton->setToolTip("Move Layer Up");
  moveLayerDownButton->setToolTip("Move Layer Down");
  playButton->setToolTip("Toggle Playing");
}

void ControlsWidget::setupLayout() {
  QHBoxLayout *layout = new QHBoxLayout{this};
  setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setAlignment(Qt::AlignLeft);
  layout->addWidget(insertLayerButton);
  layout->addWidget(removeLayerButton);
  layout->addWidget(moveLayerUpButton);
  layout->addWidget(moveLayerDownButton);
  layout->addWidget(playButton);
}

void ControlsWidget::connectSignals() {
  CONNECT(insertLayerButton, pressed, this, insertLayer);
  CONNECT(removeLayerButton, pressed, this, removeLayer);
  CONNECT(moveLayerUpButton, pressed, this, moveLayerUp);
  CONNECT(moveLayerDownButton, pressed, this, moveLayerDown);
  CONNECT(playButton, toggled, this, toggleTimer);
  CONNECT(animTimer, timeout, this, nextFrame);
}

void ControlsWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.setPen(Qt::NoPen);
  painter.setBrush(glob_border_color);
  painter.drawRect(layer_width - glob_border_width, 0, glob_border_width, cell_height);
  painter.drawRect(0, cell_height - glob_border_width, layer_width, glob_border_width);
  painter.setBrush(glob_main);
  painter.drawRect(0, 0, layer_width - glob_border_width, cell_height - glob_border_width);
}

#include "timeline controls widget.moc"
