//
//  text input widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "text input widget.hpp"

#include "connect.hpp"
#include <QtGui/qevent.h>
#include "global font.hpp"
#include <QtGui/qpainter.h>
#include "widget painting.hpp"

#ifdef Q_OS_MACOS
#include "native mac.hpp"
#endif

TextInputWidget::TextInputWidget(QWidget *parent, const WidgetRect rect)
  : QLineEdit{parent},
    rect{rect},
    cursorBlinkTimer{this} {
  setFixedSize(rect.widget().size());
  setFont(getGlobalFont());
  updateMargins();
  setFrame(false);
  setAttribute(Qt::WA_MacShowFocusRect, false);
  setMask(QRegion{rect.outer()});
  
  CONNECT(cursorBlinkTimer, timeout,               this, blink);
  CONNECT(this,             selectionChanged,      this, showCursor);
  CONNECT(this,             cursorPositionChanged, this, setOffset);
  CONNECT(this,             cursorPositionChanged, this, showCursor);
  
  cursorBlinkTimer.setInterval(box_cursor_blink_interval_ms);
  cursorBlinkTimer.start();
}

void TextInputWidget::setText(const QString &text) {
  QLineEdit::setText(text);
  offset = 0;
  repaint();
}

void TextInputWidget::blink() {
  cursorBlinkStatus = !cursorBlinkStatus;
  repaint();
}

void TextInputWidget::showCursor() {
  cursorBlinkStatus = true;
  cursorBlinkTimer.stop();
  cursorBlinkTimer.start();
  repaint();
}

void TextInputWidget::hideCursor() {
  cursorBlinkStatus = false;
  cursorBlinkTimer.stop();
  cursorBlinkTimer.start();
  repaint();
}

int TextInputWidget::getAlignOffset() const {
  if (alignment() & Qt::AlignRight) {
    return rect.inner().width() - (text().size() * glob_font_stride_px + glob_font_kern_px);
  } else if (alignment() & Qt::AlignLeft) {
    return 0;
  } else Q_UNREACHABLE();
}

int TextInputWidget::getCursorPos(const int chars) const {
  return rect.pos().x()
    + chars * glob_font_stride_px
    - glob_text_margin
    + offset
    + getAlignOffset();
}

int TextInputWidget::getMinCursorPos() const {
  return rect.pos().x() - glob_text_margin;
}

int TextInputWidget::getMaxCursorPos() const {
  return rect.inner().right() + 1 - glob_text_margin;
}

int TextInputWidget::getMinOffset() const {
  // Should we consider rect.pos()?
  return rect.inner().width()
    - glob_text_margin
    - text().length() * glob_font_stride_px;
}

void TextInputWidget::setOffset(int, const int newCursor) {
  const int newPos = getCursorPos(newCursor);
  const int min = getMinCursorPos();
  const int max = getMaxCursorPos();
  if (newPos < min) {
    offset += min - newPos;
  } else if (newPos > max) {
    offset -= newPos - max;
  }
  constrainOffset();
  updateMargins();
}

void TextInputWidget::constrainOffset() {
  // Don't use std::clamp
  if (alignment() & Qt::AlignRight) {
    offset = std::min(offset, -getMinOffset());
    offset = std::max(offset, 0);
  } else if (alignment() & Qt::AlignLeft) {
    offset = std::max(offset, getMinOffset());
    offset = std::min(offset, 0);
  } else Q_UNREACHABLE();
}

void TextInputWidget::updateMargins() {
  setTextMargins(
    rect.pos().x() + offset + getAlignOffset(),
    rect.pos().y(),
    rect.widget().right() - rect.inner().right(),
    rect.widget().bottom() - rect.inner().bottom()
  );
}

void TextInputWidget::paintBackground(QPainter &painter) {
  painter.setBrush(box_background_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(rect.inner());
}

void TextInputWidget::paintText(QPainter &painter) {
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  painter.setClipRect(rect.inner());
  QPoint textPos = rect.pos();
  textPos.rx() += offset + getAlignOffset();
  textPos.ry() += glob_font_accent_px;
  painter.drawText(textPos, text());
}

void TextInputWidget::paintCursor(QPainter &painter) {
  if (!hasFocus() || !cursorBlinkStatus || selectionStart() != -1) return;
  painter.setBrush(glob_text_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(
    getCursorPos(cursorPosition()),
    rect.inner().y(),
    box_cursor_width,
    rect.inner().height()
  );
}

void TextInputWidget::paintSelection(QPainter &painter) {
  if (!hasFocus() || selectionStart() == -1) return;
  painter.setBrush(box_selection_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(
    getCursorPos(selectionStart()),
    rect.inner().y(),
    selectionLength() * glob_font_stride_px + glob_font_kern_px,
    rect.inner().height()
  );
}

void TextInputWidget::keyPressEvent(QKeyEvent *event) {
  QLineEdit::keyPressEvent(event);
  
  // TODO: Qt bug
  // https://bugreports.qt.io/browse/QTBUG-78933
  #ifdef Q_OS_MACOS
  hideMouseUntilMouseMoves();
  #endif
}

void TextInputWidget::focusInEvent(QFocusEvent *event) {
  QLineEdit::focusInEvent(event);
  showCursor();
}

void TextInputWidget::focusOutEvent(QFocusEvent *event) {
  offset = 0;
  QLineEdit::focusOutEvent(event);
}

void TextInputWidget::wheelEvent(QWheelEvent *event) {
  offset += event->pixelDelta().x();
  constrainOffset();
  updateMargins();
  repaint();
}

void TextInputWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  paintBackground(painter);
  paintBorder(painter, rect, glob_border_color);
  paintText(painter);
  paintCursor(painter);
  paintSelection(painter);
}

#include "text input widget.moc"
