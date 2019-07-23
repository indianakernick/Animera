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

ComboBoxWidget::ComboBoxWidget(QWidget *parent, const WidgetRect rect)
  : Base{parent}, rect{rect} {
  setCursor(Qt::PointingHandCursor);
  setFont(getGlobalFont());
  setFixedSize(rect.widget().size());
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
  paintBorder(painter, rect, glob_border_color);
  const int arrowX = rect.inner().right() + 1 - arrow.width() - glob_text_padding;
  const int borderX = arrowX - glob_text_padding - glob_border_width;
  painter.fillRect(
    borderX, rect.inner().top(),
    glob_border_width, rect.inner().height(),
    glob_border_color
  );
  painter.fillRect(QRect{
    rect.inner().topLeft(),
    QPoint{borderX - 1, rect.inner().bottom()}
  }, glob_dark_1);
  painter.fillRect(
    borderX + glob_border_width, rect.inner().top(),
    glob_text_padding * 2 + arrow.width(), rect.inner().height(),
    glob_main
  );
  painter.drawPixmap(arrowX, rect.inner().top() + glob_text_padding, arrow);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  painter.setFont(getGlobalFont());
  painter.setClipRect(QRect{
    rect.contentPos(),
    QPoint{borderX - glob_text_padding, rect.inner().bottom() + 1 - glob_text_padding}
  });
  QPoint textPos = rect.contentPos();
  textPos.ry() += glob_font_accent_px;
  painter.drawText(textPos, currentText());
}
