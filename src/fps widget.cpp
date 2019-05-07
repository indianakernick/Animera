//
//  fps widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 6/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "fps widget.hpp"

#include <cmath>
#include "config.hpp"
#include "global font.hpp"
#include <QtCore/qtimer.h>
#include <QtGui/qpainter.h>

constexpr int fps_digits = 4;

FPSWidget::FPSWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedSize(
    2 * glob_text_padding + fps_digits * glob_font_stride_px - 1_px,
    2 * glob_text_padding + glob_font_px
  );
}

void FPSWidget::start() {
  timer.start();
}

void FPSWidget::paintEvent(QPaintEvent *) {
  ++frames;
  const int fps = qRound(frames / (timer.elapsed() / 1000.0));

  QPainter painter{this};
  painter.fillRect(rect(), glob_dark_2);
  painter.setFont(getGlobalFont());
  painter.setPen(glob_text_color);
  painter.drawText(
    glob_text_padding,
    glob_text_padding + glob_font_accent_px,
    QString::number(fps)
  );

  if (timer.hasExpired(10 * 1000)) {
    timer.restart();
    frames = 0;
  }
  
  if (fps > 10000) {
    QTimer::singleShot(1000, [this]{
      timer.restart();
      frames = 0;
      repaint();
    });
  } else {
    update();
  }
}
