//
//  radio button widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "radio button widget.hpp"

#include <QtGui/qevent.h>

void RadioButtonWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    nextCheckState();
  }
}

void RadioButtonWidget::mouseReleaseEvent(QMouseEvent *) {}
