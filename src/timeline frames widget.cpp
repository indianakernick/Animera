//
//  timeline frames widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 24/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
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

void FramesWidget::setFrameCount(const FrameIdx count) {
  frames = count;
  setWidth();
  repaint();
}

void FramesWidget::setMargin(const int newMargin) {
  margin = newMargin;
  setWidth();
  repaint();
}

int FramesWidget::roundUpFrames() const {
  return ((+frames + frame_incr - 1) / frame_incr) * frame_incr;
}

void FramesWidget::setWidth() {
  setFixedWidth((roundUpFrames() + frame_incr) * cell_width + margin);
}

void FramesWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  int x = 0;
  for (int f = 0; f <= roundUpFrames(); f += frame_incr) {
    painter.drawText(
      glob_text_margin + x,
      glob_text_margin + glob_font_accent_px,
      QString::number(f)
    );
    painter.fillRect(
      x - glob_border_width, 0,
      glob_border_width, height(),
      glob_border_color
    );
    x += frame_incr * cell_width;
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

FramesWidget *FrameScrollWidget::getChild() {
  auto frames = new FramesWidget{this};
  // We cannot simply call setViewportMargins
  CONNECT(this, shouldSetRightMargin, frames, setMargin);
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
