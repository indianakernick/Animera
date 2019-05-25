//
//  separator widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 25/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "separator widget.hpp"

#include "config.hpp"
#include <QtGui/qpainter.h>

HoriSeparator::HoriSeparator(QWidget *parent)
  : QWidget{parent} {
  setMinimumWidth(glob_border_width);
  setFixedHeight(glob_border_width + 2 * glob_padding);
}

void HoriSeparator::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  
  painter.setPen(Qt::NoPen);
  painter.setBrush(glob_main);
  // @TODO remove this bug workaround
  // https://bugreports.qt.io/browse/QTBUG-75783
  painter.drawRect(0, 0, width(), glob_padding);
  painter.drawRect(0, glob_padding + glob_border_width, width(), glob_padding);
  
  painter.setBrush(glob_border_color);
  painter.drawRect(0, glob_padding, width(), glob_border_width);
}

VertSeparator::VertSeparator(QWidget *parent)
  : QWidget{parent} {
  setFixedWidth(glob_border_width + 2 * glob_padding);
  setMinimumHeight(glob_border_width);
}

void VertSeparator::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  
  painter.setPen(Qt::NoPen);
  painter.setBrush(glob_main);
  // @TODO remove this bug workaround
  // https://bugreports.qt.io/browse/QTBUG-75783
  painter.drawRect(0, 0, glob_padding, height());
  painter.drawRect(glob_padding + glob_border_width, 0, glob_padding, height());
  
  painter.setBrush(glob_border_color);
  painter.drawRect(glob_padding, 0, glob_border_width, height());
}
