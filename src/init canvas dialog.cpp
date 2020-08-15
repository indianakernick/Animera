//
//  init canvas dialog.cpp
//  Animera
//
//  Created by Indiana Kernick on 3/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "init canvas dialog.hpp"

#include "connect.hpp"
#include "label widget.hpp"
#include "config colors.hpp"
#include "combo box widget.hpp"
#include "number input widget.hpp"
#include "radio button widget.hpp"
#include <QtWidgets/qgridlayout.h>
#include "text push button widget.hpp"

InitCanvasDialog::InitCanvasDialog(QWidget *parent)
  : Dialog{parent} {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("New File");
  setStyleSheet("background-color:" + glob_main.name());
  createWidgets();
  setupLayout();
  connectSignals();
}

void InitCanvasDialog::submit() {
  const Format format = static_cast<Format>(formats->currentIndex());
  const QSize size = {width->value(), height->value()};
  Q_EMIT canvasInitialized(format, size);
}

void InitCanvasDialog::createWidgets() {
  width = new NumberInputWidget{this, init_size_rect, init_size_range};
  height = new NumberInputWidget{this, init_size_rect, init_size_range};
  formats = new ComboBoxWidget{this, 9};
  formats->addItem("RGB");
  formats->addItem("Indexed");
  formats->addItem("Grayscale");
  ok = new TextPushButtonWidget{this, init_button_rect, "Create"};
  cancel = new TextPushButtonWidget{this, init_button_rect, "Cancel"};
}

void InitCanvasDialog::setupLayout() {
  auto *layout = new QGridLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(glob_margin, glob_margin, glob_margin, glob_margin);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->addWidget(makeLabel(this, 8, "Width: "), 0, 0, Qt::AlignLeft);
  layout->addWidget(width, 0, 1, Qt::AlignRight);
  layout->addWidget(makeLabel(this, 8, "Height: "), 1, 0, Qt::AlignLeft);
  layout->addWidget(height, 1, 1, Qt::AlignRight);
  layout->addWidget(makeLabel(this, 8, "Format: "), 2, 0, Qt::AlignLeft);
  layout->addWidget(formats, 2, 1, Qt::AlignRight);
  
  auto *buttonLayout = new QHBoxLayout{};
  layout->addLayout(buttonLayout, 3, 0, 1, 2, Qt::AlignCenter);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(ok);
  buttonLayout->addWidget(cancel);
  buttonLayout->addStretch();
}

void InitCanvasDialog::connectSignals() {
  CONNECT(ok,     pressed,  this, accept);
  CONNECT(cancel, pressed,  this, reject);
  CONNECT(this,   accepted, this, submit);
}

#include "init canvas dialog.moc"
