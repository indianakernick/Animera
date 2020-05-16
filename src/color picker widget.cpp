//
//  color picker widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 7/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "color picker widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "label widget.hpp"
#include "picker impl rgba.hpp"
#include "picker impl gray.hpp"

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedWidth(pick_svgraph_rect.widget().width() + 2 * glob_margin);
}

void ColorPickerWidget::initCanvas(const Format format) {
  setImpl(format);
  init();
  setupLayout();
  adjustSize();
  connectSignals();
}

void ColorPickerWidget::attach(ColorHandle *newHandle) {
  if (handle) handle->detach();
  handle = nullptr;
  if (newHandle) {
    setColor(newHandle->getInitialColor());
    nameLabel->setText(newHandle->getName());
  }
  handle = newHandle;
}

void ColorPickerWidget::setColor(const QRgb color) {
  impl->setColor(color);
}

void ColorPickerWidget::setHandleColor(const QRgb color) {
  if (handle) {
    handle->setColor(color);
  }
}

void ColorPickerWidget::setImpl(const Format format) {
  switch (format) {
    case Format::rgba:
    case Format::index:
      impl = std::make_unique<PickerImplRGBA>();
      break;
    case Format::gray:
      impl = std::make_unique<PickerImplGray>();
      break;
  }
}

void ColorPickerWidget::init() {
  nameLabel = new LabelWidget{this, pick_name_rect, "PALETTE 42"};
  impl->init(this);
}

void ColorPickerWidget::setupLayout() {
  auto *layout = new QGridLayout{this};
  layout->setAlignment(Qt::AlignTop);
  layout->setSpacing(0);
  layout->setContentsMargins(glob_margin, glob_margin, glob_margin, glob_margin);
  layout->addWidget(nameLabel, 0, 0, 1, 6);
  impl->setupLayout(layout);
}

void ColorPickerWidget::connectSignals() {
  CONNECT(impl, colorChanged,   this, setHandleColor);
  CONNECT(impl, shouldShowNorm, this, shouldShowNorm);
  impl->connectSignals();
}

#include "color picker widget.moc"
