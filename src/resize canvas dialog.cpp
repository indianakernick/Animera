//
//  resize canvas dialog.cpp
//  Animera
//
//  Created by Indiana Kernick on 27/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "resize canvas dialog.hpp"

#include "connect.hpp"
#include "label widget.hpp"
#include "config colors.hpp"
#include "config geometry.hpp"
#include <QtWidgets/qgridlayout.h>
#include "number input widget.hpp"
#include "text push button widget.hpp"

ResizeCanvasDialog::ResizeCanvasDialog(QWidget *parent)
  : Dialog{parent} {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Resize");
  setStyleSheet("background-color:" + glob_main.name());
  createWidgets();
  setupLayout();
  connectSignals();
}

void ResizeCanvasDialog::setSize(const QSize size) {
  width->setValue(size.width());
  height->setValue(size.height());
}

void ResizeCanvasDialog::submit() {
  Q_EMIT canvasResized({width->value(), height->value()});
}

void ResizeCanvasDialog::createWidgets() {
  width = new NumberInputWidget{this, init_size_rect, init_size_range};
  height = new NumberInputWidget{this, init_size_rect, init_size_range};
  ok = new TextPushButtonWidget{this, init_button_rect, "Resize"};
  cancel = new TextPushButtonWidget{this, init_button_rect, "Cancel"};
}

void ResizeCanvasDialog::setupLayout() {
  auto *layout = new QGridLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(glob_margin, glob_margin, glob_margin, glob_margin);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->addWidget(makeLabel(this, 8, "Width: "), 0, 0, Qt::AlignLeft);
  layout->addWidget(width, 0, 1, Qt::AlignRight);
  layout->addWidget(makeLabel(this, 8, "Height: "), 1, 0, Qt::AlignLeft);
  layout->addWidget(height, 1, 1, Qt::AlignRight);
  layout->addWidget(ok, 3, 0);
  layout->addWidget(cancel, 3, 1);
}

void ResizeCanvasDialog::connectSignals() {
  CONNECT(ok,     pressed,  this, accept);
  CONNECT(cancel, pressed,  this, reject);
  CONNECT(this,   accepted, this, submit);
}

#include "resize canvas dialog.moc"
