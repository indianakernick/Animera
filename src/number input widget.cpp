//
//  number input widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 18/6/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "number input widget.hpp"

#include "connect.hpp"
#include <QtGui/qevent.h>

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

NumberInputWidget::NumberInputWidget(
  QWidget *parent,
  const WidgetRect &rect,
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
  }
  TextInputWidget::keyPressEvent(event);
}

#include "number input widget.moc"
