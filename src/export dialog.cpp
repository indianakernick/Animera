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
}

void ExportDialog::finalize() {
  Q_EMIT exportSprite(options);
}

void ExportDialog::updateFormatItems(const int layerIdx) {
  if (format == Format::palette) {
    if (layerIdx == 0 /* All (composed) */ && formatSelect->count() == 3) {
      formatSelect->removeItem(2); // Grayscale
      formatSelect->removeItem(1); // Indexed
    } else if (formatSelect->count() == 1) {
      formatSelect->addItem("Indexed");
      formatSelect->addItem("Grayscale");
    }
  }
}

void ExportDialog::setLayerSelect(const int layerIdx) {
  options.layerSelect = static_cast<LayerSelect>(layerIdx);
}

void ExportDialog::setFrameSelect(const int frameIdx) {
  options.frameSelect = static_cast<FrameSelect>(frameIdx);
}

void ExportDialog::setExportFormat(const int formatIdx) {
  switch (format) {
    case Format::rgba:
      options.format = ExportFormat::rgba;
      break;
    case Format::palette:
      if (formatSelect->count() == 1) {
        options.format = ExportFormat::rgba;
      } else if (formatSelect->count() == 3) {
        options.format = static_cast<ExportFormat>(formatIdx);
      } else Q_UNREACHABLE();
      break;
    case Format::gray:
      options.format = static_cast<ExportFormat>(formatIdx + 2);
      break;
    default: Q_UNREACHABLE();
  }
}

void ExportDialog::addFormatOptions() {
  switch (format) {
    case Format::rgba:
      formatSelect->addItem("RGBA");
      break;
    case Format::palette:
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
  // @TODO validator for export name
  name->setText("sprite_%000F");
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

template <size_t Size>
LabelWidget *makeLabel(QWidget *parent, const char (&text)[Size]) {
  return new LabelWidget{parent, textBoxRect(Size - 1), text};
}

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
  CONNECT_SETTER(  name,         textChanged,         options.name);
  CONNECT_SETTER(  dir,          pathChanged,         options.directory);
  CONNECT_SETTER(  layerStride,  valueChanged,        options.layerLine.stride);
  CONNECT_SETTER(  layerOffset,  valueChanged,        options.layerLine.offset);
  CONNECT_SETTER(  frameStride,  valueChanged,        options.frameLine.stride);
  CONNECT_SETTER(  frameOffset,  valueChanged,        options.frameLine.offset);
  CONNECT_OVERLOAD(layerSelect,  currentIndexChanged, this, setLayerSelect, int);
  CONNECT_OVERLOAD(frameSelect,  currentIndexChanged, this, setFrameSelect, int);
  CONNECT_OVERLOAD(formatSelect, currentIndexChanged, this, setExportFormat, int);
  CONNECT_OVERLOAD(layerSelect,  currentIndexChanged, this, updateFormatItems, int);
  CONNECT(         ok,           pressed,             this, accept);
  CONNECT(         cancel,       pressed,             this, reject);
  CONNECT(         this,         accepted,            this, finalize);
}

void ExportDialog::initDefault() {
  // @TODO maybe create widgets, connect signals, set default values on widgets
  options.name = name->text();
  options.directory = QDir::homePath();
  options.layerLine.stride = expt_stride.def;
  options.layerLine.offset = expt_offset.def;
  options.frameLine.stride = expt_stride.def;
  options.frameLine.offset = expt_offset.def;
  options.layerSelect = LayerSelect::all_composited;
  options.frameSelect = FrameSelect::all;
  setExportFormat(0);
}

#include "export dialog.moc"
