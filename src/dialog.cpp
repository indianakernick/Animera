//
//  dialog.cpp
//  Animera
//
//  Created by Indiana Kernick on 3/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "dialog.hpp"

#include <QtGui/qevent.h>

Dialog::Dialog(QWidget *parent)
  : QDialog{parent} {
  setFocusPolicy(Qt::ClickFocus);
}

void Dialog::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Return) {
    accept();
  } else if (event->key() == Qt::Key_Escape) {
    reject();
  }
}
