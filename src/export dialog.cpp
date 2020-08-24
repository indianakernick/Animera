//
//  export dialog.cpp
//  Animera
//
//  Created by Indiana Kernick on 20/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "export dialog.hpp"

#include "connect.hpp"
#include "settings.hpp"
#include <QtCore/qdir.h>
#include <QtGui/qevent.h>
#include "label widget.hpp"
#include "config colors.hpp"
#include "export params.hpp"
#include "combo box widget.hpp"
#include "file input widget.hpp"
#include <QtWidgets/qboxlayout.h>
#include "png atlas generator.hpp"
#include <QtWidgets/qgridlayout.h>
#include "number input widget.hpp"
#include "text push button widget.hpp"

ExportDialog::ExportDialog(QWidget *parent, const Format format)
  : Dialog{parent}, format{format} {
  setWindowTitle("Export");
  setStyleSheet("background-color:" + glob_main.name());
  createWidgets();
  setupLayout();
  connectSignals();
}

void ExportDialog::setPath(const QString &path) {
  if (name->text().isEmpty()) {
    name->setText(nameFromPath(path));
  }
}

void ExportDialog::setInfo(const ExportAnimationInfo &newInfo) {
  info = newInfo;
}

namespace {

const char *formatNames[] = {
  "RGBA",
  "Indexed",
  "Gray",
  "Gray-Alpha",
  "Monochrome"
};

PixelFormat formatFromString(const QString &format) {
  for (std::size_t f = 0; f != std::size(formatNames); ++f) {
    if (format == formatNames[f]) {
      return static_cast<PixelFormat>(f);
    }
  }
  Q_UNREACHABLE();
}

QString formatToString(const PixelFormat format) {
  return formatNames[static_cast<std::size_t>(format)];
}

}

void ExportDialog::submit() {
  ExportParams params;
  params.name = "";
  params.directory = dir->path();
  params.pixelFormat = formatFromString(formatSelect->currentText());
  params.generator = std::make_unique<PngAtlasGenerator>();
  params.whitepixel = true;
  
  AnimExportParams &animParams = params.anims.emplace_back();
  
  animParams.name.baseName = name->text();
  animParams.name.layerName = LayerNameMode{layerName->currentIndex()};
  animParams.name.groupName = GroupNameMode{groupName->currentIndex()};
  animParams.name.frameName = FrameNameMode{frameName->currentIndex()};
  
  animParams.transform.scaleX = scaleX->value();
  animParams.transform.scaleY = scaleY->value();
  animParams.transform.angle = rotate->currentIndex();
  
  animParams.layers = selectLayers(info, LayerSelection{layerSelect->currentIndex()});
  animParams.frames = selectFrames(info, FrameSelection{frameSelect->currentIndex()});
  
  animParams.composite = composite->currentText() == "Enabled";
  
  Q_EMIT exportAnimation(params);
}

void ExportDialog::updateFormatItems(const QString &compositeStr) {
  if (format == Format::index) {
    if (compositeStr == "Enabled" && formatSelect->count() == 3) {
      formatSelect->clearWithItem(formatToString(PixelFormat::rgba));
    } else if (formatSelect->count() == 1) {
      formatSelect->clearWithItem(formatToString(PixelFormat::index));
      formatSelect->addItem(formatToString(PixelFormat::gray));
      formatSelect->addItem(formatToString(PixelFormat::monochrome));
    }
  }
}

void ExportDialog::addFormatOptions() {
  switch (format) {
    case Format::rgba:
      formatSelect->addItem(formatToString(PixelFormat::rgba));
      break;
    case Format::index:
      formatSelect->addItem(formatToString(PixelFormat::rgba));
      break;
    case Format::gray:
      formatSelect->addItem(formatToString(PixelFormat::gray_alpha));
      formatSelect->addItem(formatToString(PixelFormat::gray));
      formatSelect->addItem(formatToString(PixelFormat::monochrome));
      break;
    default: Q_UNREACHABLE();
  }
}

void ExportDialog::createWidgets() {
  name = new TextInputWidget{this, textBoxRect(16)};
  
  dir = new FileInputWidget{this, 55};
  dir->setPath(getDirSettings(pref_export_dir));
  
  layerName = new ComboBoxWidget{this, 11};
  layerName->addItem("Auto layer");
  layerName->addItem("Layer name");
  layerName->addItem("Layer index");
  layerName->addItem("Empty");
  
  groupName = new ComboBoxWidget{this, 11};
  groupName->addItem("Auto group");
  groupName->addItem("Group name");
  groupName->addItem("Group index");
  groupName->addItem("Empty");
  
  frameName = new ComboBoxWidget{this, 14};
  frameName->addItem("Auto frame");
  frameName->addItem("Relative frame");
  frameName->addItem("Absolute frame");
  frameName->addItem("Empty");
  frameName->addItem("Sheet column");
  frameName->addItem("Sheet row");
  
  layerSelect = new ComboBoxWidget{this, 8};
  layerSelect->addItem("Visible");
  layerSelect->addItem("Hidden");
  layerSelect->addItem("All");
  layerSelect->addItem("Current");
  layerSelect->addItem("Selected");
  
  frameSelect = new ComboBoxWidget{this, 8};
  frameSelect->addItem("All");
  frameSelect->addItem("Current");
  frameSelect->addItem("Selected");
  
  composite = new ComboBoxWidget{this, 8};
  composite->addItem("Enabled");
  composite->addItem("Disabled");
  
  scaleX = new NumberInputWidget{this, textBoxRect(3), expt_scale, true};
  scaleY = new NumberInputWidget{this, textBoxRect(3), expt_scale, true};
  
  rotate = new ComboBoxWidget{this, 10};
  rotate->addItem("0");
  rotate->addItem("90");
  rotate->addItem("180");
  rotate->addItem("270");
  
  formatSelect = new ComboBoxWidget{this, 11};
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
  dirLayout->addSpacing(1_px);
  dirLayout->addWidget(dir);
  dirLayout->addStretch();
  
  auto *nameLayout = makeLayout<QHBoxLayout>(layout);
  nameLayout->addWidget(makeLabel(this, "Name: "));
  nameLayout->addWidget(name);
  nameLayout->addWidget(layerName);
  nameLayout->addWidget(groupName);
  nameLayout->addWidget(frameName);
  
  auto *firstLayout = makeLayout<QHBoxLayout>(layout);
  firstLayout->addWidget(makeLabel(this, "Composite: "));
  firstLayout->addWidget(composite);
  firstLayout->addStretch();
  firstLayout->addWidget(makeLabel(this, "Layers: "));
  firstLayout->addWidget(layerSelect);
  firstLayout->addStretch();
  firstLayout->addWidget(makeLabel(this, "Scale: X "));
  firstLayout->addWidget(scaleX);
  firstLayout->addWidget(makeLabel(this, " Y "));
  firstLayout->addWidget(scaleY);
  
  auto *secondLayout = makeLayout<QHBoxLayout>(layout);
  secondLayout->addWidget(makeLabel(this, "Format: "));
  secondLayout->addWidget(formatSelect);
  secondLayout->addStretch();
  secondLayout->addWidget(makeLabel(this, "Frames: "));
  secondLayout->addWidget(frameSelect);
  secondLayout->addStretch();
  secondLayout->addWidget(makeLabel(this, "Rotate: "));
  secondLayout->addSpacing(2_px);
  secondLayout->addWidget(rotate);
  
  auto *buttonLayout = makeLayout<QHBoxLayout>(layout);
  buttonLayout->addStretch();
  buttonLayout->addWidget(ok);
  buttonLayout->addWidget(cancel);
  buttonLayout->addStretch();
}

void ExportDialog::connectSignals() {
  CONNECT(composite,  currentTextChanged, this, updateFormatItems);
  CONNECT(ok,         pressed,            this, accept);
  CONNECT(cancel,     pressed,            this, reject);
  CONNECT(this,       accepted,           this, submit);
  CONNECT_LAMBDA(dir, pathChanged,        setDirSettings(pref_export_dir));
}

#include "export dialog.moc"
