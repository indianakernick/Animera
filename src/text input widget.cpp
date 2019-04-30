//
//  text input widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "text input widget.hpp"

#include "connect.hpp"
#include "global font.hpp"
#include <QtGui/qpainter.h>
#include "widget painting.hpp"

TextInputWidget::TextInputWidget(QWidget *parent, const WidgetRect rect)
  : QLineEdit{parent},
    cursorBlinkTimer{this},
    rect{rect} {
  setFixedSize(rect.widget().size());
  setFont(getGlobalFont());
  setTextMargins(
    rect.contentPos().x(),
    rect.contentPos().y(),
    rect.inner().x(),
    rect.inner().y()
  );
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
  painter.drawRect(rect.inner());
}

void TextInputWidget::renderText(QPainter &painter) {
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  QPoint textPos = rect.contentPos();
  textPos.ry() += glob_font_accent_px;
  painter.drawText(textPos, text());
}

void TextInputWidget::renderCursor(QPainter &painter) {
  if (!hasFocus() || !cursorBlinkStatus || selectionStart() != -1) return;
  painter.setBrush(glob_text_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(QRect{
    rect.contentPos().x() + cursorPosition() * glob_font_stride_px - glob_text_padding,
    rect.inner().y(),
    box_cursor_width,
    rect.inner().height()
  });
}

void TextInputWidget::renderSelection(QPainter &painter) {
  if (!hasFocus() || selectionStart() == -1) return;
  painter.setBrush(box_selection_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(QRect{
    rect.contentPos().x() + selectionStart() * glob_font_stride_px - glob_text_padding,
    rect.inner().y(),
    selectionLength() * glob_font_stride_px + glob_font_kern_px,
    rect.inner().height()
  });
}

void TextInputWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  renderBackground(painter);
  paintBorder(painter, rect, glob_border_color);
  renderText(painter);
  renderCursor(painter);
  renderSelection(painter);
}

#include "text input widget.moc"
