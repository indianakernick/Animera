//
//  export dialog.cpp
//  Animera
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export dialog.hpp"

#include "connect.hpp"
#include <QtCore/qdir.h>
#include <QtGui/qevent.h>
#include "label widget.hpp"
#include "export pattern.hpp"
#include "export options.hpp"
#include "combo box widget.hpp"
#include "file input widget.hpp"
#include "color input widget.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qgridlayout.h>
#include "text push button widget.hpp"

ExportDialog::ExportDialog(QWidget *parent, const ExportSpriteInfo info)
  : Dialog{parent}, info{info} {
  setWindowTitle("Export");
  setStyleSheet("background-color:" + glob_main.name());
  createWidgets();
  setupLayout();
  connectSignals();
}

void ExportDialog::setCurrent(const CellPos pos) {
  current = pos;
}

void ExportDialog::setSelection(const CellRect rect) {
  selection = rect;
}

namespace {

ExportFormat formatFromString(const QString &format) {
         if (format == "RGBA") {
    return ExportFormat::rgba;
  } else if (format == "Indexed") {
    return ExportFormat::index;
  } else if (format == "Gray") {
    return ExportFormat::gray;
  } else if (format == "Gray Alpha") {
    return ExportFormat::gray_alpha;
  } else if (format == "Monochrome") {
    return ExportFormat::monochrome;
  } else Q_UNREACHABLE();
}

}

void ExportDialog::submit() {
  ExportOptions options;
  options.name = name->text();
  options.directory = dir->path();
  
  const QString layerStr = layerSelect->currentText();
         if (layerStr == "All") {
    options.selection.minL = LayerIdx{};
    options.selection.maxL = info.layers - LayerIdx{1};
  } else if (layerStr == "Current") {
    options.selection.minL = options.selection.maxL = current.l;
  } else if (layerStr == "Selected") {
    options.selection.minL = selection.minL;
    options.selection.maxL = selection.maxL;
  } else Q_UNREACHABLE();
  
  const QString frameStr = frameSelect->currentText();
         if (frameStr == "All") {
    options.selection.minF = FrameIdx{};
    options.selection.maxF = info.frames - FrameIdx{1};
  } else if (layerStr == "Current") {
    options.selection.minF = options.selection.maxF = current.f;
  } else if (layerStr == "Selected") {
    options.selection.minF = selection.minF;
    options.selection.maxF = selection.maxF;
  } else Q_UNREACHABLE();
  
  options.layerLine.stride = LayerIdx{layerStride->value()};
  options.layerLine.offset = LayerIdx{layerOffset->value()};
  options.frameLine.stride = FrameIdx{frameStride->value()};
  options.frameLine.offset = FrameIdx{frameOffset->value()};
  options.format = formatFromString(formatSelect->currentText());
  options.scaleX = scaleX->value();
  options.scaleY = scaleY->value();
  options.angle = rotate->currentIndex();
  Q_EMIT exportSprite(options);
}

void ExportDialog::updateFormatItems(const QString &compositeStr) {
  if (info.format == Format::index) {
    if (compositeStr == "Yes" && formatSelect->count() == 3) {
      formatSelect->clearWithItem("RGBA");
    } else if (formatSelect->count() == 1) {
      formatSelect->clearWithItem("Indexed");
      formatSelect->addItem("Gray");
      formatSelect->addItem("Monochrome");
    }
  }
}

void ExportDialog::addFormatOptions() {
  switch (info.format) {
    case Format::rgba:
      formatSelect->addItem("RGBA");
      break;
    case Format::index:
      formatSelect->addItem("RGBA");
      break;
    case Format::gray:
      formatSelect->addItem("Gray Alpha");
      formatSelect->addItem("Gray");
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
  layerStride = new NumberInputWidget{this, textBoxRect(3), expt_stride, true};
  layerOffset = new NumberInputWidget{this, textBoxRect(3), expt_offset};
  frameStride = new NumberInputWidget{this, textBoxRect(3), expt_stride, true};
  frameOffset = new NumberInputWidget{this, textBoxRect(3), expt_offset};
  layerSelect = new ComboBoxWidget{this, 14};
  layerSelect->addItem("All");
  layerSelect->addItem("Current");
  layerSelect->addItem("Selected");
  frameSelect = new ComboBoxWidget{this, 14};
  frameSelect->addItem("All");
  frameSelect->addItem("Current");
  frameSelect->addItem("Selected");
  composite = new ComboBoxWidget{this, 10};
  composite->addItem("Yes");
  composite->addItem("No");
  scaleX = new NumberInputWidget{this, textBoxRect(3), expt_scale, true};
  scaleY = new NumberInputWidget{this, textBoxRect(3), expt_scale, true};
  rotate = new ComboBoxWidget{this, 14};
  rotate->addItem("0");
  rotate->addItem("90");
  rotate->addItem("180");
  rotate->addItem("270");
  formatSelect = new ComboBoxWidget{this, 14};
  addFormatOptions();
  ok = new TextPushButtonWidget{this, textBoxRect(8), "Export"};
  cancel = new TextPushButtonWidget{this, textBoxRect(8), "Cancel"};
}

namespace {

template <typename Layout>
Layout *makeLayout(QBoxLayout *parent) {
  auto *layout = new Layout;
  parent->addLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  return layout;
}

}

void ExportDialog::setupLayout() {
  auto *layout = new QVBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(glob_margin, glob_margin, glob_margin, glob_margin);
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
  
  auto *transformLayout = makeLayout<QHBoxLayout>(layout);
  transformLayout->addWidget(makeLabel(this, "Scale - X: "));
  transformLayout->addWidget(scaleX);
  transformLayout->addWidget(makeLabel(this, " Y: "));
  transformLayout->addWidget(scaleY);
  transformLayout->addStretch();
  transformLayout->addWidget(makeLabel(this, "Rotate: "));
  transformLayout->addWidget(rotate);
  
  auto *compositeLayout = makeLayout<QHBoxLayout>(layout);
  compositeLayout->addWidget(makeLabel(this, "Composite: "));
  compositeLayout->addSpacing(2_px);
  compositeLayout->addWidget(composite);
  compositeLayout->addStretch();
  
  auto *buttonLayout = makeLayout<QHBoxLayout>(layout);
  buttonLayout->addStretch();
  buttonLayout->addWidget(ok);
  buttonLayout->addWidget(cancel);
  buttonLayout->addStretch();
}

void ExportDialog::connectSignals() {
  CONNECT(composite, currentTextChanged, this, updateFormatItems);
  CONNECT(ok,        pressed,            this, accept);
  CONNECT(cancel,    pressed,            this, reject);
  CONNECT(this,      accepted,           this, submit);
}

#include "export dialog.moc"
