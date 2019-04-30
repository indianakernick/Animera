//
//  color label widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color label widget.hpp"

#include "global font.hpp"
#include <QtGui/qpainter.h>

LabelWidget::LabelWidget(
  QWidget *parent,
  const QString &text,
  const WidgetRect rect
) : QWidget{parent}, text{text}, rect{rect} {
  setFixedSize(rect.widget().size());
}

void LabelWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  QPoint textPos = rect.inner().topLeft();
  textPos += QPoint{0, glob_font_accent_px};
  textPos += toPoint(glob_text_padding);
  painter.drawText(textPos, text);
}
