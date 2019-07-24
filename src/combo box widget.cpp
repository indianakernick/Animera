//
//  combo box widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "combo box widget.hpp"

#include "global font.hpp"
#include <QtGui/qpainter.h>
#include "widget painting.hpp"

ComboBoxWidget::ComboBoxWidget(QWidget *parent, const int chars)
  : QComboBox{parent}, rects{textBoxIconRect(chars)} {
  setCursor(Qt::PointingHandCursor);
  setFont(getGlobalFont());
  setFixedSize(rects.widget().size());
  arrow = bakeColoredBitmap(":/General/up down arrow.pbm", glob_light_2);
  // setItemDelegate
  
  //setFrame(false);
  /*setStyleSheet(
    "QComboBox:on QWidget {"
      "background-color: " + glob_dark_1.name() + ";"
      "color: " + glob_text_color.name() + ";"
      "border-width: " + QString::number(glob_border_width) + "px;"
      "border-color: " + glob_border_color.name() + ";"
      "border-style: solid;"
    "}"
  );*/
  /*setStyleSheet(
    "QComboBox {"
      "background-color: " + glob_dark_1.name() + ";"
      "color: " + glob_text_color.name() + ";"
      "border-width: " + QString::number(glob_border_width) + "px;"
      "border-color: " + glob_border_color.name() + ";"
      "border-style: solid;"
    "}"
    
    "QComboBox::drop-down {"
      "border-left-width: " + QString::number(glob_border_width) + "px;"
      "border-left-color: " + glob_border_color.name() + ";"
      "border-left-style: solid;"
    "}"
  );*/
}

void ComboBoxWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  paintBorder(painter, rects.text(), glob_border_color);
  paintBorder(painter, rects.icon(), glob_border_color);
  painter.fillRect(rects.border(), glob_border_color);
  painter.fillRect(rects.textInner(), glob_dark_1);
  painter.fillRect(rects.iconInner(), glob_main);
  painter.drawPixmap(rects.iconPos(), arrow);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  painter.setFont(getGlobalFont());
  painter.setClipRect(rects.textInner());
  QPoint textPos = rects.textPos();
  textPos.ry() += glob_font_accent_px;
  painter.drawText(textPos, currentText());
}
