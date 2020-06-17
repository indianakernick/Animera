//
//  text push button widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 20/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "text push button widget.hpp"

#include "global font.hpp"
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include "config colors.hpp"
#include "config geometry.hpp"

TextPushButtonWidget::TextPushButtonWidget(
  QWidget *parent, const WidgetRect &rect, const QString &text
) : QAbstractButton{parent}, rect{rect}, pixmap{rect.widget().size()} {
  setFixedSize(rect.widget().size());
  setCursor(Qt::PointingHandCursor);
 
  QBitmap bitmap = QBitmap{":/General/text button.png"};
  bitmap = bitmap.scaled(bitmap.size() * glob_scale);
  assert(bitmap.size() == rect.outer().size());
  
  QRegion region = bitmap;
  region.translate(rect.outer().topLeft());
  setMask(region);
  
  QPainter painter{&pixmap};
  painter.fillRect(rect.outer(), glob_light_1);
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  painter.drawText(rect.inner(), Qt::AlignCenter, text);
}

void TextPushButtonWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.drawPixmap(rect.widget(), pixmap);
}
