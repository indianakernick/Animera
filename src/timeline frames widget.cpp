//
//  timeline frames widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline frames widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "global font.hpp"
#include <QtGui/qpainter.h>

FramesWidget::FramesWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedSize(0, cell_height);
}

void FramesWidget::appendFrame() {
  ++frames;
  setFixedWidth((roundUpFrames() + frame_incr) * cell_icon_step + margin);
  Q_EMIT widthChanged(width());
  repaint();
}

void FramesWidget::setMargin(const int newMargin) {
  margin = newMargin;
  setFixedWidth((roundUpFrames() + frame_incr) * cell_icon_step + margin);
  repaint();
}

int FramesWidget::roundUpFrames() const {
  return ((frames + 1) / frame_incr) * frame_incr;
}

void FramesWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  int x = 0;
  for (int f = 0; f <= roundUpFrames(); f += frame_incr) {
    painter.drawText(x + 1_px, 2_px + glob_font_accent_px, QString::number(f));
    painter.fillRect(
      x - glob_border_width, 0,
      glob_border_width, height(),
      glob_border_color
    );
    x += frame_incr * cell_icon_step;
  }
  painter.fillRect(
    0, height() - glob_border_width,
    width(), glob_border_width,
    glob_border_color
  );
}

FrameScrollWidget::FrameScrollWidget(QWidget *parent)
  : QScrollArea{parent} {
  setFrameShape(NoFrame);
  setFixedHeight(cell_height);
  setStyleSheet("background-color:" + glob_dark_2.name());
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

FramesWidget *FrameScrollWidget::setChild(FramesWidget *frames) {
  // We cannot simply call setViewportMargins
  CONNECT(this, changeRightMargin, frames, setMargin);
  setWidget(frames);
  return frames;
}

void FrameScrollWidget::paintEvent(QPaintEvent *) {
  QPainter painter{viewport()};
  painter.fillRect(
    0, height() - glob_border_width,
    width(), glob_border_width,
    glob_border_color
  );
}

#include "timeline frames widget.moc"
