//
//  radio button widget.cpp
//  Pixel 2
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

// Radio buttons don't seem to work properly without this
// Clicking them only sometimes checks them
void RadioButtonWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    nextCheckState();
  }
}

void RadioButtonWidget::mouseReleaseEvent(QMouseEvent *) {}
