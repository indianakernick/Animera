//
//  picker impl gray.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "picker impl gray.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "label widget.hpp"
#include "color input widget.hpp"
#include "color slider widget.hpp"

void PickerImplGray::init(QWidget *parent) {
  gray = pick_default_gray;
  graySlider = new GraySliderWidget{parent};
  boxY = new NumberInputWidget{parent, pick_number_rect, {0, 255, gray}};
  boxHex = new HexInputWidget{parent, pick_hex_rect, {gray, gray, gray}, 255};
  boxHex->setReadOnly(true);
  labelY = new LabelWidget{parent, pick_label_rect, "Y"};
  labelHex = new LabelWidget{parent, pick_label_rect, "#"};
}

void PickerImplGray::setupLayout(QGridLayout *layout) {
  layout->addWidget(graySlider, 1, 0, 1, 6);
  layout->addWidget(labelY,     2, 0);
  layout->addWidget(boxY,       2, 1);
  layout->addWidget(labelHex,   2, 2);
  layout->addWidget(boxHex,     2, 3, 1, 3);
}

void PickerImplGray::connectSignals() {
  CONNECT(graySlider,  grayChanged,  boxY,        changeValue);
  CONNECT(boxY,        valueChanged, graySlider,  changeGray);

  CONNECT(graySlider,  grayChanged,  this,        changeGray);
  CONNECT(boxY,        valueChanged, this,        changeGray);
  
  //CONNECT(boxHex,      rgbaChanged,  this,        changeRGBAtoGray);
}

void PickerImplGray::setColor(const QRgb color) {
  gray = color & 255;
  graySlider->changeGray(gray);
  boxY->changeValue(gray);
  changeGray(gray);
}

void PickerImplGray::changeGray(const int newGray) {
  gray = newGray;
  boxHex->changeRgba({gray, gray, gray}, 255);
  Q_EMIT colorChanged(gray);
}
