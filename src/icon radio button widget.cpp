//
//  icon radio button widget.cpp
//  Animera
//
//  Created by Indi Kernick on 15/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "icon radio button widget.hpp"

#include <QtGui/qpainter.h>

IconRadioButtonWidget::IconRadioButtonWidget(
  QWidget *parent,
  QPixmap onPix,
  QPixmap offPix
) : RadioButtonWidget{parent},
    onPix{onPix},
    offPix{offPix} {
  assert(onPix.size() == offPix.size());
  setFixedSize(onPix.size());
}

void IconRadioButtonWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.drawPixmap(rect(), isChecked() ? onPix : offPix);
}
