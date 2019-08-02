//
//  radio button widget.cpp
//  Animera
//
//  Created by Indi Kernick on 3/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "radio button widget.hpp"

#include <QtGui/qevent.h>

RadioButtonWidget::RadioButtonWidget(QWidget *parent)
  : QAbstractButton{parent} {
  setCursor(Qt::PointingHandCursor);
  setCheckable(true);
  setAutoExclusive(true);
}

void RadioButtonWidget::uncheck() {
  setAutoExclusive(false);
  setChecked(false);
  setAutoExclusive(true);
}

void RadioButtonWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    nextCheckState();
  }
}

void RadioButtonWidget::mouseReleaseEvent(QMouseEvent *) {}
