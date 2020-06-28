//
//  scroll bar widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 21/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "scroll bar widget.hpp"

#include <QtGui/qpainter.h>
#include "config colors.hpp"
#include "config geometry.hpp"

namespace {

int documentLength(const QScrollBar *bar) {
  return bar->maximum() - bar->minimum() + bar->pageStep();
}

int scrollRange(const QScrollBar *bar) {
  return bar->maximum() - bar->minimum();
}

int pagePixels(const QScrollBar *bar, const int pixelLength) {
  return (pixelLength * bar->pageStep()) / documentLength(bar);
}

int scrollRangePixels(const QScrollBar *bar, const int pixelLength) {
  return pixelLength - pagePixels(bar, pixelLength);
}

int valuePixels(const QScrollBar *bar, const int pixelLength) {
  if (bar->minimum() == bar->maximum()) {
    return 0;
  } else {
    return (scrollRangePixels(bar, pixelLength) * bar->value()) / scrollRange(bar);
  }
}

}

ScrollBarWidget::ScrollBarWidget(Qt::Orientation orient, QWidget *parent)
  : QScrollBar{orient, parent} {
  const QString sizeStr = QString::number(glob_scroll_width);
  setStyleSheet("width:" + sizeStr + ";height:" + sizeStr);
}
  
void ScrollBarWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(rect(), glob_scroll_back);
  if (orientation() == Qt::Horizontal) {
    painter.fillRect(QRect{
      valuePixels(this, width()),
      0,
      pagePixels(this, width()),
      height()
    }, glob_scroll_handle);
  } else if (orientation() == Qt::Vertical) {
    painter.fillRect(QRect{
      0,
      valuePixels(this, height()),
      width(),
      pagePixels(this, height())
    }, glob_scroll_handle);
  } else {
    Q_UNREACHABLE();
  }
}

ScrollCornerWidget::ScrollCornerWidget(QWidget *parent)
  : QWidget{parent} {}
  
void ScrollCornerWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(rect(), glob_scroll_corner);
}

ScrollAreaWidget::ScrollAreaWidget(QWidget *parent)
  : QScrollArea{parent} {
  setVerticalScrollBar(new ScrollBarWidget{Qt::Vertical, this});
  setHorizontalScrollBar(new ScrollBarWidget{Qt::Horizontal, this});
  corner = new ScrollCornerWidget{this};
  corner->hide();
}

void ScrollAreaWidget::adjustMargins() {
  if (rightMargin() && bottomMargin()) {
    setCornerWidget(corner);
    corner->show();
  } else {
    setCornerWidget(nullptr);
    corner->hide();
  }
}

int ScrollAreaWidget::rightMargin() const {
  return height() < widget()->height() ? glob_scroll_width : 0;
}

int ScrollAreaWidget::bottomMargin() const {
  return width() < widget()->width() ? glob_scroll_width : 0;
}
