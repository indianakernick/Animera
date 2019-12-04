//
//  status bar widget.cpp
//  Animera
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "status bar widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "strings.hpp"
#include "scope time.hpp"
#include "global font.hpp"
#include <QtGui/qpainter.h>

StatusBarWidget::StatusBarWidget(QWidget *parent)
  : QWidget{parent} {
  timer.setInterval(stat_temp_duration_ms);
  timer.setSingleShot(true);
  CONNECT(timer, timeout, this, hideTemp);
  setMinimumWidth(stat_min_width);
  setFixedHeight(stat_height);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

void StatusBarWidget::showTemp(const std::string_view text) {
  tempText = toLatinString(text);
  timer.start();
  repaint();
}

void StatusBarWidget::showNorm(const std::string_view text) {
  SCOPE_TIME("StatusBarWidget::showNorm");
  
  normText = toLatinString(text);
  repaint();
}

void StatusBarWidget::showPerm(const std::string_view text) {
  permText = toLatinString(text);
  repaint();
}

void StatusBarWidget::paintEvent(QPaintEvent *) {
  SCOPE_TIME("StatusBarWidget::paintEvent");

  QPainter painter{this};
  painter.fillRect(rect(), stat_background);
  painter.setFont(getGlobalFont());
  painter.setPen(glob_text_color);
  const QPoint pos = {
    glob_margin + glob_text_margin,
    glob_margin + glob_text_margin + glob_font_accent_px
  };
  if (!tempText.isEmpty()) {
    painter.drawText(pos, tempText);
    return;
  }
  if (!normText.isEmpty()) {
    painter.drawText(pos, normText);
    return;
  }
  if (!permText.isEmpty()) {
    painter.drawText(pos, permText);
  }
}

void StatusBarWidget::hideTemp() {
  tempText.clear();
  repaint();
}

#include "status bar widget.moc"
