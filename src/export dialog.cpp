//
//  export dialog.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export dialog.hpp"

#include "connect.hpp"
#include "label widget.hpp"
#include "combo box widget.hpp"
#include "color input widget.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qgridlayout.h>
#include "text push button widget.hpp"

ExportDialog::ExportDialog(QWidget *parent)
  : QDialog{parent} {
  setWindowTitle("Export");
  setStyleSheet("background-color:" + glob_main.name());
  setFocusPolicy(Qt::ClickFocus);
  createWidgets();
  setupLayout();
  connectSignals();
}

void ExportDialog::finalize() {
  Q_EMIT exportSprite(options);
}

void ExportDialog::createWidgets() {
  name = new TextInputWidget{this, textBoxRect(16, 0)};
  name->setText("sprite_%000F");
  layerStride = new NumberInputWidget{this, textBoxRect(3, 0), expt_stride};
  layerOffset = new NumberInputWidget{this, textBoxRect(3, 0), expt_offset};
  frameStride = new NumberInputWidget{this, textBoxRect(3, 0), expt_stride};
  frameOffset = new NumberInputWidget{this, textBoxRect(3, 0), expt_offset};
  layerSelect = new ComboBoxWidget{this, comboBoxRect(14, 0)};
  layerSelect->addItem("All (composed)");
  layerSelect->addItem("All");
  layerSelect->addItem("Current");
  frameSelect = new ComboBoxWidget{this, comboBoxRect(14, 0)};
  frameSelect->addItem("All");
  frameSelect->addItem("Current");
  ok = new TextPushButtonWidget{this, textBoxRect(8, 0), "Ok"};
  cancel = new TextPushButtonWidget{this, textBoxRect(8, 0), "Cancel"};
}

namespace {

template <size_t Size>
LabelWidget *makeLabel(QWidget *parent, const char (&text)[Size]) {
  return new LabelWidget{parent, textBoxRect(Size - 1, 0), text};
}

}

void ExportDialog::setupLayout() {
  auto *layout = new QVBoxLayout{this};
  setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(glob_padding, glob_padding, glob_padding, glob_padding);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  
  auto *nameLayout = new QHBoxLayout{};
  layout->addLayout(nameLayout);
  nameLayout->setSpacing(0);
  nameLayout->setContentsMargins(0, 0, 0, 0);
  nameLayout->addWidget(makeLabel(this, "Name: "));
  nameLayout->addStretch();
  nameLayout->addWidget(name);
  
  auto *lineLayout = new QGridLayout{};
  layout->addLayout(lineLayout);
  lineLayout->setSpacing(0);
  lineLayout->setContentsMargins(0, 0, 0, 0);
  lineLayout->addWidget(makeLabel(this, "L = layer * "), 0, 0);
  lineLayout->addWidget(layerStride, 0, 1);
  lineLayout->addWidget(makeLabel(this, " + "), 0, 2);
  lineLayout->addWidget(layerOffset, 0, 3);
  lineLayout->addWidget(makeLabel(this, "F = frame * "), 1, 0);
  lineLayout->addWidget(frameStride, 1, 1);
  lineLayout->addWidget(makeLabel(this, " + "), 1, 2);
  lineLayout->addWidget(frameOffset, 1, 3);
  
  auto *selectLayout = new QGridLayout{};
  layout->addLayout(selectLayout);
  selectLayout->setSpacing(0);
  selectLayout->setContentsMargins(0, 0, 0, 0);
  selectLayout->addWidget(makeLabel(this, "Layers: "), 0, 0);
  selectLayout->addWidget(layerSelect, 0, 1);
  selectLayout->addWidget(makeLabel(this, "Frames: "), 1, 0);
  selectLayout->addWidget(frameSelect, 1, 1);
  
  auto *buttonLayout = new QHBoxLayout{};
  layout->addLayout(buttonLayout);
  buttonLayout->setSpacing(0);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(ok);
  buttonLayout->addWidget(cancel);
  buttonLayout->addStretch();
}

void ExportDialog::connectSignals() {
  CONNECT(ok,     pressed,  this, accept);
  CONNECT(cancel, pressed,  this, reject);
  CONNECT(this,   accepted, this, finalize);
}

#include "export dialog.moc"
