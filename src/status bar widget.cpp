//
//  status bar widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "status bar widget.hpp"

#include "config.hpp"
#include "global font.hpp"
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

namespace {

QLatin1String toLatinString(const std::string_view text) {
  return QLatin1String{text.data(), static_cast<int>(text.size())};
}

}

void StatusBarWidget::showTemp(const std::string_view text) {
  tempText = toLatinString(text);
  timer.start();
  repaint();
}

void StatusBarWidget::showPerm(const std::string_view text) {
  permText = toLatinString(text);
  repaint();
}

void StatusBarWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(rect(), stat_background);
  painter.setFont(getGlobalFont());
  const QPoint pos = {stat_padding, glob_font_accent_px + stat_padding};
  if (tempText.isEmpty()) {
    painter.drawText(pos, permText);
  } else if (permText.isEmpty()) {
    painter.drawText(pos, tempText);
  } else {
    painter.drawText(pos, permText + " | " + tempText);
  }
  painter.setPen(stat_text);
}

void StatusBarWidget::hideTemp() {
  tempText.clear();
  repaint();
}

#include "status bar widget.moc"
