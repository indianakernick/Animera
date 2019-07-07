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
#include <QtGui/qpainter.h>

ControlsWidget::ControlsWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedSize(layer_width, cell_height);
  animTimer.setInterval(100);
  CONNECT(&animTimer, timeout, this, nextFrame);
}

void ControlsWidget::toggleAnimation() {
  if (animTimer.isActive()) {
    animTimer.stop();
  } else {
    animTimer.start();
  }
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
