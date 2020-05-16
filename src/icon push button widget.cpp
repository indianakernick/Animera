//
//  icon push button widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "icon push button widget.hpp"

#include <QtGui/qpainter.h>

IconPushButtonWidget::IconPushButtonWidget(QWidget *parent, QPixmap pix)
  : QAbstractButton{parent}, pix{pix} {
  setCursor(Qt::PointingHandCursor);
  setFixedSize(pix.size());
}

void IconPushButtonWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.drawPixmap(rect(), pix);
}
