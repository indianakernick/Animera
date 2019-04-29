//
//  color label widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color label widget.hpp"

#include "config.hpp"
#include "global font.hpp"
#include <QtGui/qpainter.h>

ColorLabelWidget::ColorLabelWidget(QWidget *parent, const QString &text)
  : QWidget{parent}, text{text} {
  setFixedSize(pick_label_size.widget().size());
}

void ColorLabelWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(pick_label_text_color);
  QPoint textPos = pick_label_size.inner().topLeft();
  textPos.ry() += glob_font_accent_px;
  textPos += QPoint{2_px, 1_px};
  painter.drawText(textPos, text);
}
