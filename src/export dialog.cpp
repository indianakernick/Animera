//
//  export dialog.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export dialog.hpp"

#include "connect.hpp"
#include <QtCore/qdir.h>
#include "label widget.hpp"
#include "export pattern.hpp"
#include "export options.hpp"
#include "combo box widget.hpp"
#include "file input widget.hpp"
#include "color input widget.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qgridlayout.h>
#include "text push button widget.hpp"

ExportDialog::ExportDialog(QWidget *parent, const Format format)
  : QDialog{parent}, format{format} {
  setWindowTitle("Export");
  setStyleSheet("background-color:" + glob_main.name());
  setFocusPolicy(Qt::ClickFocus);
  createWidgets();
  setupLayout();
  connectSignals();
  // @TODO remember previous settings
}

void ExportDialog::submit() {
  ExportOptions options;
  options.name = name->text();
  options.directory = dir->path();
  options.layerSelect = static_cast<LayerSelect>(layerSelect->currentIndex());
  options.frameSelect = static_cast<FrameSelect>(frameSelect->currentIndex());
  options.layerLine.stride = layerStride->value();
  options.layerLine.offset = layerOffset->value();
  options.frameLine.stride = frameStride->value();
  options.frameLine.offset = frameOffset->value();
  const QString formatStr = formatSelect->currentText();
         if (formatStr == "RGBA") {
    options.format = ExportFormat::rgba;
  } else if (formatStr == "Indexed") {
    options.format = ExportFormat::indexed;
  } else if (formatStr == "Grayscale") {
    options.format = ExportFormat::grayscale;
  } else if (formatStr == "Monochrome") {
    options.format = ExportFormat::monochrome;
  } else Q_UNREACHABLE();
  Q_EMIT exportSprite(options);
}

void ExportDialog::updateFormatItems(const int layerIdx) {
  if (format == Format::index) {
    if (layerIdx == static_cast<int>(LayerSelect::all_composited) && formatSelect->count() == 2) {
      formatSelect->clear();
      formatSelect->addItem("RGBA");
    } else if (formatSelect->count() == 1) {
      formatSelect->clear();
      formatSelect->addItem("Indexed");
      formatSelect->addItem("Grayscale");
    }
  }
}

void ExportDialog::addFormatOptions() {
  switch (format) {
    case Format::rgba:
      formatSelect->addItem("RGBA");
      break;
    case Format::index:
      formatSelect->addItem("RGBA");
      break;
    case Format::gray:
      formatSelect->addItem("Grayscale");
      formatSelect->addItem("Monochrome");
      break;
    default: Q_UNREACHABLE();
  }
}

void ExportDialog::createWidgets() {
  name = new TextInputWidget{this, textBoxRect(16)};
  auto *validator = new ExportPatternValidator{name};
  name->setValidator(validator);
  name->setText(validator->defaultText());
  dir = new FileInputWidget{this, 40};
  layerStride = new NumberInputWidget{this, textBoxRect(3), expt_stride};
  layerOffset = new NumberInputWidget{this, textBoxRect(3), expt_offset};
  frameStride = new NumberInputWidget{this, textBoxRect(3), expt_stride};
  frameOffset = new NumberInputWidget{this, textBoxRect(3), expt_offset};
  layerSelect = new ComboBoxWidget{this, 14};
  layerSelect->addItem("All (composed)");
  layerSelect->addItem("All");
  layerSelect->addItem("Current");
  frameSelect = new ComboBoxWidget{this, 14};
  frameSelect->addItem("All");
  frameSelect->addItem("Current");
  formatSelect = new ComboBoxWidget{this, 14};
  addFormatOptions();
  ok = new TextPushButtonWidget{this, textBoxRect(8), "Ok"};
  cancel = new TextPushButtonWidget{this, textBoxRect(8), "Cancel"};
  setFocus();
}

namespace {

template <typename Layout>
Layout *makeLayout(QBoxLayout *parent) {
  auto layout = new Layout;
  parent->addLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  return layout;
}

}

void ExportDialog::setupLayout() {
  auto *layout = new QVBoxLayout{this};
  setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(glob_padding, glob_padding, glob_padding, glob_padding);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  
  auto *dirLayout = makeLayout<QHBoxLayout>(layout);
  dirLayout->addWidget(makeLabel(this, "Folder: "));
  dirLayout->addWidget(dir);
  dirLayout->addStretch();
  
  auto *nameLayout = makeLayout<QHBoxLayout>(layout);
  nameLayout->addWidget(makeLabel(this, "Name: "));
  nameLayout->addSpacing(4_px);
  nameLayout->addWidget(name);
  nameLayout->addStretch();
  nameLayout->addWidget(makeLabel(this, "Format: "));
  nameLayout->addWidget(formatSelect);
  
  auto *layerLayout = makeLayout<QHBoxLayout>(layout);
  layerLayout->addWidget(makeLabel(this, "L = layer * "));
  layerLayout->addWidget(layerStride);
  layerLayout->addWidget(makeLabel(this, " + "));
  layerLayout->addWidget(layerOffset);
  layerLayout->addStretch();
  layerLayout->addWidget(makeLabel(this, "Layers: "));
  layerLayout->addWidget(layerSelect);
  
  auto *frameLayout = makeLayout<QHBoxLayout>(layout);
  frameLayout->addWidget(makeLabel(this, "F = frame * "));
  frameLayout->addWidget(frameStride);
  frameLayout->addWidget(makeLabel(this, " + "));
  frameLayout->addWidget(frameOffset);
  frameLayout->addStretch();
  frameLayout->addWidget(makeLabel(this, "Frames: "));
  frameLayout->addWidget(frameSelect);
  
  auto *buttonLayout = makeLayout<QHBoxLayout>(layout);
  buttonLayout->addStretch();
  buttonLayout->addWidget(ok);
  buttonLayout->addWidget(cancel);
  buttonLayout->addStretch();
}

void ExportDialog::connectSignals() {
  CONNECT_OVERLOAD(layerSelect,  currentIndexChanged, this, updateFormatItems, int);
  CONNECT(         ok,           pressed,             this, accept);
  CONNECT(         cancel,       pressed,             this, reject);
  CONNECT(         this,         accepted,            this, submit);
}

#include "export dialog.moc"
