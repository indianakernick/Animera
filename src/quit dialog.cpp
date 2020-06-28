//
//  quit dialog.cpp
//  Animera
//
//  Created by Indiana Kernick on 8/12/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "quit dialog.hpp"

#include "connect.hpp"
#include "label widget.hpp"
#include "config colors.hpp"
#include <QtWidgets/qboxlayout.h>
#include "text push button widget.hpp"

QuitDialog::QuitDialog(QWidget *parent)
  : Dialog{parent} {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Quit");
  setStyleSheet("background-color:" + glob_main.name());
  createWidgets();
  setupLayout();
  connectSignals();
}

void QuitDialog::createWidgets() {
  quit = new TextPushButtonWidget{this, textBoxRect(8), "Quit"};
  save = new TextPushButtonWidget{this, textBoxRect(8), "Save"};
  cancel = new TextPushButtonWidget{this, textBoxRect(8), "Cancel"};
}

void QuitDialog::setupLayout() {
  auto *layout = new QVBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(glob_margin, glob_margin, glob_margin, glob_margin);
  
  layout->addWidget(makeLabel(this, "Quit without saving?"));
  
  auto *buttonLayout = new QHBoxLayout;
  layout->addLayout(buttonLayout);
  buttonLayout->setSpacing(0);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  
  buttonLayout->addWidget(quit);
  buttonLayout->addWidget(save);
  buttonLayout->addWidget(cancel);
}

void QuitDialog::connectSignals() {
  CONNECT(quit, pressed, this, shouldQuit);
  CONNECT(quit, pressed, this, accept);
  CONNECT(save, pressed, this, shouldSave);
  CONNECT(save, pressed, this, accept);
  CONNECT(cancel, pressed, this, shouldCancel);
  CONNECT(cancel, pressed, this, accept);
}

#include "quit dialog.moc"
