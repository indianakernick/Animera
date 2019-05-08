//
//  status bar widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "status bar widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "global font.hpp"
#include <QtGui/qpainter.h>

StatusBarWidget::StatusBarWidget(QWidget *parent)
  : QWidget{parent} {
  timer.setInterval(stat_temp_duration_ms);
  timer.setSingleShot(true);
  CONNECT(&timer, timeout, this, hideTemp);
  setMinimumWidth(stat_min_width);
  setFixedHeight(stat_height);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
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
  painter.setPen(glob_text_color);
  const QPoint pos = {
    glob_padding + glob_text_padding,
    glob_padding + glob_text_padding + glob_font_accent_px
  };
  if (tempText.isEmpty()) {
    painter.drawText(pos, permText);
  } else if (permText.isEmpty()) {
    painter.drawText(pos, tempText);
  } else {
    painter.drawText(pos, permText + " | " + tempText);
  }
}

void StatusBarWidget::hideTemp() {
  tempText.clear();
  repaint();
}

#include "status bar widget.moc"
