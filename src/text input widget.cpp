//
//  text input widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "text input widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "global font.hpp"
#include <QtGui/qpainter.h>
#include "widget painting.hpp"

TextInputWidget::TextInputWidget(QWidget *parent, const RectWidgetSize size, const int offsetX)
  : QLineEdit{parent},
    cursorBlinkTimer{this},
    boxSize{size},
    offsetX{offsetX} {
  setFixedSize(boxSize.widget().size());
  setFont(getGlobalFont());
  const int margin = boxSize.padding() + boxSize.border();
  setTextMargins(margin + offsetX, margin, margin, margin);
  setFrame(false);
  setAttribute(Qt::WA_MacShowFocusRect, 0);
  
  CONNECT(&cursorBlinkTimer, timeout,               this, blinkSlot);
  CONNECT(this,              selectionChanged,      this, showCursor);
  CONNECT(this,              cursorPositionChanged, this, showCursor);
  
  cursorBlinkTimer.setInterval(box_cursor_blink_interval_ms);
  cursorBlinkTimer.setTimerType(Qt::CoarseTimer);
  cursorBlinkTimer.start();
}

void TextInputWidget::blinkSlot() {
  cursorBlinkStatus = !cursorBlinkStatus;
  repaint();
}
  
void TextInputWidget::showCursor() {
  cursorBlinkStatus = true;
  cursorBlinkTimer.stop();
  cursorBlinkTimer.start();
  repaint();
}

void TextInputWidget::focusInEvent(QFocusEvent *event) {
  QLineEdit::focusInEvent(event);
  QTimer::singleShot(0, this, &QLineEdit::selectAll);
}

void TextInputWidget::renderBackground(QPainter &painter) {
  painter.setBrush(box_background_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(boxSize.inner());
}

void TextInputWidget::renderText(QPainter &painter) {
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(box_text_color);
  QPoint textPos = boxSize.inner().topLeft();
  textPos += QPoint{offsetX, glob_font_accent_px};
  textPos += QPoint{box_text_padding, box_text_padding};
  painter.drawText(textPos, text());
}

void TextInputWidget::renderCursor(QPainter &painter) {
  if (!hasFocus() || !cursorBlinkStatus || selectionStart() != -1) return;
  painter.setBrush(box_cursor_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(QRect{
    boxSize.inner().x() + cursorPosition() * glob_font_stride_px + offsetX,
    boxSize.inner().y(),
    box_cursor_width,
    boxSize.inner().height()
  });
}

void TextInputWidget::renderSelection(QPainter &painter) {
  if (!hasFocus() || selectionStart() == -1) return;
  painter.setBrush(box_selection_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(QRect{
    boxSize.inner().x() + selectionStart() * glob_font_stride_px + offsetX,
    boxSize.inner().y(),
    box_text_padding + selectionLength() * glob_font_stride_px,
    boxSize.inner().height()
  });
}

void TextInputWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  renderBackground(painter);
  paintBorder(painter, boxSize, box_border_color);
  renderText(painter);
  renderCursor(painter);
  renderSelection(painter);
}

#include "text input widget.moc"
