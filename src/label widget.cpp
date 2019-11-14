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
  setAttribute(Qt::WA_StaticContents);
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
  const QPoint pos = rect.pos() - rect.inner().topLeft();
  const QRect bounds = rect.inner().translated(pos);
  painter.drawText(bounds, Qt::AlignTop | Qt::AlignLeft, text);
}

QSize wrapToWidth(QString &text, const int width) {
  assert(width > 0);
  int line = 0;
  int white = -1;
  QSize size = {0, 1};
  for (int i = 0; i != text.size(); ++i) {
    if (text[i] == QChar::LineFeed) {
      size.setWidth(std::max(size.width(), i - line));
      line = i + 1;
      ++size.rheight();
      continue;
    } else if (text[i] == QChar::Space) {
      white = i;
      continue;
    }
    while (i - line >= width) {
      int firstWhite = white;
      if (firstWhite > 0) {
        while (text[firstWhite - 1] == QChar::Space) --firstWhite;
      }
      if (white >= 0 && firstWhite - line <= width) {
        size.setWidth(std::max(size.width(), firstWhite - line));
        text[white] = QChar::LineFeed;
        line = white + 1;
      } else {
        size.setWidth(width);
        text.insert(line + width, QChar::LineFeed);
        ++i;
        line += width + 1;
      }
      white = -1;
      ++size.rheight();
    }
  }
  size.setWidth(std::max(size.width(), text.size() - line));
  return size;
}
