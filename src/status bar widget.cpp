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
  tempTimer.setInterval(stat_temp_duration_ms);
  tempTimer.setSingleShot(true);
  CONNECT(tempTimer, timeout, this, hideTemp);
  apndTimer.setInterval(stat_temp_duration_ms);
  apndTimer.setSingleShot(true);
  CONNECT(apndTimer, timeout, this, hideApnd);
  setMinimumWidth(stat_rect.widget().width());
  setFixedHeight(stat_rect.widget().height());
}

void StatusBarWidget::showTemp(const std::string_view text) {
  tempText = toLatinString(text);
  tempTimer.start();
  repaint();
}

void StatusBarWidget::showNorm(const std::string_view text) {
  normText = toLatinString(text);
  repaint();
}

void StatusBarWidget::showPerm(const std::string_view text) {
  permText = toLatinString(text);
  repaint();
}

void StatusBarWidget::showApnd(const std::string_view text) {
  apndText = toLatinString(text);
  apndTimer.start();
  repaint();
}

void StatusBarWidget::paintEvent(QPaintEvent *) {
  SCOPE_TIME("StatusBarWidget::paintEvent");

  QPainter painter{this};
  painter.fillRect(rect(), stat_background);
  painter.setFont(getGlobalFont());
  painter.setPen(glob_text_color);
  const QPoint pos = stat_rect.pos() + QPoint{0, glob_font_accent_px};
  
  if (!tempText.isEmpty()) {
    painter.drawText(pos, tempText);
  } else if (!normText.isEmpty()) {
    painter.drawText(pos, normText);
    if (!apndText.isEmpty()) {
      const int normWidth = normText.size() * glob_font_stride_px;
      const int apndOffset = normWidth + 3 * glob_font_stride_px;
      const int lineX = pos.x() + normWidth + glob_font_stride_px + 2_px;
      painter.drawText(pos + QPoint{apndOffset, 0}, apndText);
      painter.fillRect(lineX, 0, 1_px, height(), glob_text_color);
    }
  } else if (!permText.isEmpty()) {
    painter.drawText(pos, permText);
  }
}

void StatusBarWidget::hideTemp() {
  tempText.clear();
  repaint();
}

void StatusBarWidget::hideApnd() {
  apndText.clear();
  repaint();
}

#include "status bar widget.moc"
