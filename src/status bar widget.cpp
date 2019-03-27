//
//  status bar widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "status bar widget.hpp"

#include "config.hpp"
#include "render text.hpp"
#include <QtGui/qpainter.h>

StatusBarWidget::StatusBarWidget(QWidget *parent)
  : QWidget{parent} {
  timer.setInterval(stat_temp_duration_ms);
  timer.setSingleShot(true);
  connect(&timer, &QTimer::timeout, this, &StatusBarWidget::hideTemp);
  setMinimumWidth(stat_min_width);
  setFixedHeight(stat_height);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  setContentsMargins(0, 0, 0, 0);
}

void StatusBarWidget::showTemp(const std::string_view text) {
  tempText = text;
  timer.start();
  repaint();
}

void StatusBarWidget::showPerm(const std::string_view text) {
  permText = text;
  repaint();
}

void StatusBarWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(rect(), stat_background);
  if (tempText.empty()) {
    renderText(painter, stat_padding, stat_padding, permText);
  } else {
    renderText(painter, stat_padding, stat_padding, permText + " | " + tempText);
  }
}

void StatusBarWidget::hideTemp() {
  tempText.clear();
  repaint();
}

#include "status bar widget.moc"
