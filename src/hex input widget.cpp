//
//  hex input widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "hex input widget.hpp"

#include "connect.hpp"

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

HexInputWidget::HexInputWidget(
  QWidget *parent, const WidgetRect &rect, const RGB rgb, const int a
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

#include "hex input widget.moc"
