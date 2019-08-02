//
//  label widget.cpp
//  Animera
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "label widget.hpp"

#include "global font.hpp"
#include <QtGui/qpainter.h>

LabelWidget::LabelWidget(
  QWidget *parent,
  const WidgetRect rect,
  const QString &text
) : QWidget{parent}, text{text}, rect{rect} {
  setFixedSize(rect.widget().size());
}

void LabelWidget::setText(const QString &newText) {
  text = newText;
  repaint();
}

void LabelWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  QPoint textPos = rect.pos();
  textPos.ry() += glob_font_accent_px;
  painter.drawText(textPos, text);
}
