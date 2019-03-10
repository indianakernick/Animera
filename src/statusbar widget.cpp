//
//  statusbar widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "statusbar widget.hpp"

StatusBarWidget::StatusBarWidget(QWidget *parent)
  : QLabel{parent} {
  timer.setInterval(5000);
  timer.setSingleShot(true);
  connect(&timer, &QTimer::timeout, this, &StatusBarWidget::hideTemp);
  setMinimumWidth(400);
}

void StatusBarWidget::showTemp(const QString &text) {
  tempText = text;
  timer.start();
  updateText();
}

void StatusBarWidget::showPerm(const QString &text) {
  permText = text;
  updateText();
}

void StatusBarWidget::updateText() {
  if (tempText.isEmpty()) {
    setText(permText);
  } else {
    setText(permText + " | " + tempText);
  }
}

void StatusBarWidget::hideTemp() {
  tempText = "";
  updateText();
}

#include "statusbar widget.moc"
