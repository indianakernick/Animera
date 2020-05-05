//
//  color input widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "color input widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qevent.h>

#ifdef Q_OS_MACOS
#include "native mac.hpp"
#endif

NumberValidator::NumberValidator(QWidget *parent, const IntRange range, const bool skipZero)
  : QIntValidator{range.min, range.max, parent}, skipZero{skipZero} {}

void NumberValidator::fixup(QString &input) const {
  input = lastValidValue;
}

QValidator::State NumberValidator::validate(QString &input, int &pos) const {
  if (skipZero && input == "0") {
    return State::Invalid;
  }
  return QIntValidator::validate(input, pos);
}

void NumberValidator::updateValidValue(const QString &value) {
  lastValidValue = value;
}

HexValidator::HexValidator(QWidget *parent)
  : QValidator{parent} {}

void HexValidator::fixup(QString &input) const {
  // Why does this remind me of Fizz-Buzz?
  if (input.size() == 3) {
    input.insert(0, input[0]);
    input.insert(2, input[2]);
    input.insert(4, input[4]);
    input.append('F');
    input.append('F');
  } else if (input.size() == 4) {
    input.insert(0, input[0]);
    input.insert(2, input[2]);
    input.insert(4, input[4]);
    input.insert(6, input[6]);
  } else if (input.size() == 6) {
    input.append('F');
    input.append('F');
  } else {
    input = lastValidValue;
  }
}

QValidator::State HexValidator::validate(QString &input, int &pos) const {
  if (input.front() == '#') {
    input.remove(0, 1);
    pos = std::max(pos - 1, 0);
  }
  if (input.size() > 8) return State::Invalid;
  for (QChar &ch : input) {
    const char latin1 = ch.toLatin1();
    if ('0' <= latin1 && latin1 <= '9') continue;
    if ('A' <= latin1 && latin1 <= 'F') continue;
    if ('a' <= latin1 && latin1 <= 'f') {
      ch = ch.toUpper();
      continue;
    }
    return State::Invalid;
  }
  if (input.size() == 8) {
    return State::Acceptable;
  } else {
    return State::Intermediate;
  }
}

void HexValidator::updateValidValue(const QString &value) {
  lastValidValue = value;
}

NumberInputWidget::NumberInputWidget(
  QWidget *parent,
  const WidgetRect rect,
  const IntRange range,
  const bool skipZero
) : TextInputWidget{parent, rect},
    boxValidator{parent, range, skipZero},
    skipZero{skipZero} {
  setAlignment(Qt::AlignRight);
  setValidator(&boxValidator);
  setValue(range.def);
  CONNECT(this, textEdited,      this, textChanged);
  CONNECT(this, editingFinished, this, newValidValue);
}

int NumberInputWidget::value() const {
  return val;
}

void NumberInputWidget::setValue(const int num) {
  val = num;
  setText(QString::number(val));
  boxValidator.updateValidValue(text());
}

void NumberInputWidget::textChanged() {
  if (!hasAcceptableInput()) return;
  const int newValue = text().toInt();
  if (val != newValue) {
    val = newValue;
    Q_EMIT valueChanged(val);
  }
}

void NumberInputWidget::newValidValue() {
  if (hasAcceptableInput()) {
    textChanged(); // text may have been fixed up
    boxValidator.updateValidValue(text());
  }
}

void NumberInputWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Up) {
    int newVal;
    if (skipZero && val == -1) {
      newVal = 1;
    } else {
      newVal = val + 1;
    }
    setText(QString::number(std::min(newVal, boxValidator.top())));
    textChanged();
  } else if (event->key() == Qt::Key_Down) {
    int newVal;
    if (skipZero && val == 1) {
      newVal = -1;
    } else {
      newVal = val - 1;
    }
    setText(QString::number(std::max(newVal, boxValidator.bottom())));
    textChanged();
  } else {
    return TextInputWidget::keyPressEvent(event);
  }
  
  // TODO: Qt bug
  // https://bugreports.qt.io/browse/QTBUG-78933
  #ifdef Q_OS_MACOS
  hideMouseUntilMouseMoves();
  #endif
}

HexInputWidget::HexInputWidget(
  QWidget *parent, const WidgetRect rect, const RGB rgb, const int a
) : TextInputWidget{parent, rect}, boxValidator{parent} {
  setValidator(&boxValidator);
  setRgba(rgb, a);
  CONNECT(this, textEdited,      this, textChanged);
  CONNECT(this, editingFinished, this, newValidValue);
}

namespace {

QString toString(const RGB rgb, const int a) {
  const uint number = (rgb.r << 24) |
                      (rgb.g << 16) |
                      (rgb.b <<  8) |
                       a;
  QString str = QString::number(number, 16);
  while (str.size() < 8) {
    str.prepend("0");
  }
  return str;
}

void fromString(const QString &string, RGB &rgb, int &a) {
  uint number = string.toUInt(nullptr, 16);
  rgb.r =  number >> 24;
  rgb.g = (number >> 16) & 255;
  rgb.b = (number >>  8) & 255;
  a     =  number        & 255;
}

}

void HexInputWidget::setRgba(const RGB newRgb, const int newAlpha) {
  rgb = newRgb;
  alpha = newAlpha;
  setText(toString(rgb, alpha));
  boxValidator.updateValidValue(text());
}

void HexInputWidget::textChanged() {
  if (!hasAcceptableInput()) return;
  RGB newRgb;
  int newAlpha;
  fromString(text(), newRgb, newAlpha);
  if (rgb.r != newRgb.r || rgb.g != newRgb.g || rgb.b != newRgb.b || alpha != newAlpha) {
    rgb = newRgb;
    alpha = newAlpha;
    Q_EMIT rgbaChanged(rgb, alpha);
  }
}

void HexInputWidget::newValidValue() {
  if (hasAcceptableInput()) {
    textChanged(); // text may have been fixed up
    boxValidator.updateValidValue(text());
  }
}

#include "color input widget.moc"
