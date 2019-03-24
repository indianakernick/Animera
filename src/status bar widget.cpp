//
//  status bar widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "status bar widget.hpp"

#include "render text.hpp"
#include <QtGui/qpainter.h>

StatusBarWidget::StatusBarWidget(QWidget *parent)
  : QWidget{parent} {
  timer.setInterval(5000);
  timer.setSingleShot(true);
  connect(&timer, &QTimer::timeout, this, &StatusBarWidget::hideTemp);
  setMinimumWidth(400);
  setFixedHeight(22);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  setContentsMargins(0, 0, 0, 0);
  showPerm("Hello");
  showTemp("World");
}

void StatusBarWidget::showTemp(const std::string &text) {
  tempText = text;
  timer.start();
  repaint();
}

void StatusBarWidget::showPerm(const std::string &text) {
  permText = text;
  repaint();
}

void StatusBarWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(rect(), {127, 127, 127});
  if (tempText.empty()) {
    renderText(painter, 2, 2, permText);
  } else {
    renderText(painter, 2, 2, permText + " | " + tempText);
  }
}

void StatusBarWidget::hideTemp() {
  tempText = "";
  repaint();
}

#include "status bar widget.moc"
