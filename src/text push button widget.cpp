//
//  text push button widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "text push button widget.hpp"

#include "global font.hpp"
#include <QtGui/qpainter.h>

TextPushButtonWidget::TextPushButtonWidget(
  QWidget *parent, const WidgetRect rect, const QString &text
) : QAbstractButton{parent}, rect{rect}, text{text} {
  setFixedSize(rect.widget().size());
  setCursor(Qt::PointingHandCursor);
}

void TextPushButtonWidget::paintEvent(QPaintEvent *) {
  // @TODO bake
  // @TODO round corners
  QPainter painter{this};
  painter.fillRect(rect.outer(), glob_light_1);
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  painter.drawText(rect.inner(), Qt::AlignCenter, text);
}
