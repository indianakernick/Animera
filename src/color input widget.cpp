//
//  color input widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color input widget.hpp"

#include "config.hpp"
#include "connect.hpp"

NumberValidator::NumberValidator(QWidget *parent, const int max)
  : QIntValidator{0, max, parent} {}

void NumberValidator::fixup(QString &input) const {
  input = lastValidValue;
}

void NumberValidator::updateValidValue(const QString &value) {
  lastValidValue = value;
}

HexValidator::HexValidator(QWidget *parent)
  : QValidator{parent} {}

void HexValidator::fixup(QString &input) const {
  if (input.size() == 6) {
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

NumberInputWidget::NumberInputWidget(QWidget *parent, const int defaultValue, const int max)
  : TextInputWidget{parent, pick_number_size, 0}, boxValidator{parent, max} {
  setValidator(&boxValidator);
  changeValue(defaultValue);
  CONNECT(this, textEdited,      this, textChanged);
  CONNECT(this, editingFinished, this, newValidValue);
}

void NumberInputWidget::changeValue(const int num) {
  value = num;
  setText(QString::number(value));
  boxValidator.updateValidValue(text());
}

void NumberInputWidget::textChanged() {
  if (!hasAcceptableInput()) return;
  const int newValue = text().toInt();
  if (value != newValue) {
    value = newValue;
    Q_EMIT valueChanged(value);
  }
}

void NumberInputWidget::newValidValue() {
  if (hasAcceptableInput()) {
    boxValidator.updateValidValue(text());
  }
}

HexInputWidget::HexInputWidget(QWidget *parent, const QRgb defaultValue)
  : TextInputWidget{parent, pick_hex_size, 2_px}, boxValidator{parent} {
  setValidator(&boxValidator);
  changeRgba(defaultValue);
  CONNECT(this, textEdited, this, textChanged);
  CONNECT(this, editingFinished, this, newValidValue);
}

namespace {

QString toString(const QRgb color) {
  const uint number = (qRed(color)   << 24) |
                      (qGreen(color) << 16) |
                      (qBlue(color)  <<  8) |
                       qAlpha(color);
  QString str = QString::number(number, 16);
  while (str.size() < 8) {
    str.prepend("0");
  }
  return str;
}

QRgb fromString(const QString &string) {
  uint number = string.toUInt(nullptr, 16);
  return qRgba(
     number >> 24,
    (number >> 16) & 255,
    (number >>  8) & 255,
     number        & 255
  );
}

}

void HexInputWidget::changeRgba(const QRgb color) {
  value = color;
  setText(toString(value));
  boxValidator.updateValidValue(text());
}

void HexInputWidget::textChanged() {
  if (!hasAcceptableInput()) return;
  const QRgb newValue = fromString(text());
  if (value != newValue) {
    value = newValue;
    Q_EMIT rgbaChanged(value);
  }
}

void HexInputWidget::newValidValue() {
  if (hasAcceptableInput()) {
    boxValidator.updateValidValue(text());
  }
}

#include "color input widget.moc"
