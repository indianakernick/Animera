//
//  label widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "label widget.hpp"

#include "global font.hpp"
#include <QtGui/qpainter.h>
#include "config colors.hpp"

LabelWidget::LabelWidget(
  QWidget *parent,
  const WidgetRect &rect,
  const QString &text
) : QWidget{parent}, text{text}, rect{rect} {
  setFixedSize(rect.widget().size());
  setAttribute(Qt::WA_StaticContents);
}

void LabelWidget::setText(const QString &newText) {
  text = newText;
  update();
}

void LabelWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  const QRect bounds = {rect.pos(), rect.inner().size()};
  painter.drawText(bounds, Qt::AlignTop | Qt::AlignLeft, text);
}

QSize wrapToWidth(QString &text, const int width, const WrapMode mode) {
  assert(width > 0);
  int line = 0;
  int white = -1;
  QSize size = {0, 1};
  const bool canWordWrap = mode == WrapMode::word || mode == WrapMode::word_or_anywhere;
  const bool canWrapAnywhere = mode == WrapMode::anywhere || mode == WrapMode::word_or_anywhere;
  
  for (int i = 0; i != text.size(); ++i) {
    if (text[i] == QChar::LineFeed) {
      size.setWidth(std::max(size.width(), i - line));
      ++size.rheight();
      line = i + 1;
      white = -1;
      continue;
    } else if (text[i] == QChar::Space) {
      white = i;
      continue;
    }
    
    if (mode == WrapMode::none) continue;
    
    while (i - line >= width) {
      int firstWhite = white;
      while (firstWhite > 0 && text[firstWhite - 1] == QChar::Space) --firstWhite;
      
      if (canWordWrap && firstWhite >= line && firstWhite - line <= width) {
        size.setWidth(std::max(size.width(), firstWhite - line));
        text[white] = QChar::LineFeed;
        line = white + 1;
      } else if (canWrapAnywhere) {
        size.setWidth(width);
        text.insert(line + width, QChar::LineFeed);
        ++i;
        line += width + 1;
      } else break;
      
      white = -1;
      ++size.rheight();
    }
  }
  size.setWidth(std::max(size.width(), text.size() - line));
  return size;
}
