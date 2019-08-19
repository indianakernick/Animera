//
//  picker impl gray.cpp
//  Animera
//
//  Created by Indi Kernick on 10/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "picker impl gray.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "formats.hpp"
#include "label widget.hpp"
#include "color input widget.hpp"
#include "color slider widget.hpp"

namespace {

constexpr int toValue(const double gray) {
  return 100 * gray / 255 + 0.5;
}

constexpr int toGray(const double value) {
  return 255 * value / 100 + 0.5;
}

}

void PickerImplGray::init(QWidget *parent) {
  gray = pick_default_gray;
  alpha = pick_default_color.alpha();
  const int value = toValue(gray);
  graySlider = new GraySliderWidget{parent};
  alphaSlider = new AlphaSliderWidget{parent};
  alphaSlider->changeHSV({0, 0, value});
  boxA = new NumberInputWidget{parent, pick_number_rect, {0, 255, alpha}};
  boxY = new NumberInputWidget{parent, pick_number_rect, {0, 255, gray}};
  boxV = new NumberInputWidget{parent, pick_number_rect, {0, 100, value}};
  labelA = new LabelWidget{parent, pick_label_rect, "A"};
  labelY = new LabelWidget{parent, pick_label_rect, "Y"};
  labelV = new LabelWidget{parent, pick_label_rect, "V"};
}

void PickerImplGray::setupLayout(QGridLayout *layout) {
  layout->addWidget(graySlider,  1, 0, 1, 6);
  layout->addWidget(alphaSlider, 2, 0, 1, 6);
  layout->addWidget(labelA,      3, 0);
  layout->addWidget(boxA,        3, 1);
  layout->addWidget(labelY,      3, 2);
  layout->addWidget(boxY,        3, 3);
  layout->addWidget(labelV,      3, 4);
  layout->addWidget(boxV,        3, 5);
}

void PickerImplGray::connectSignals() {
  CONNECT(graySlider,  grayChanged,  boxY,        changeValue);
  CONNECT(boxY,        valueChanged, graySlider,  changeGray);

  CONNECT(graySlider,  grayChanged,  this,        setGray);
  CONNECT(boxY,        valueChanged, this,        setGray);
  
  CONNECT(alphaSlider, alphaChanged, boxA,        changeValue);
  CONNECT(boxA,        valueChanged, alphaSlider, changeAlpha);
  
  CONNECT(alphaSlider, alphaChanged, this,        setAlpha);
  CONNECT(boxA,        valueChanged, this,        setAlpha);
  
  CONNECT(boxV,        valueChanged, this,        setValue);
}

void PickerImplGray::setColor(const QRgb color) {
  gray = FormatYA::toGray(color);
  alpha = FormatYA::toAlpha(color);
  graySlider->changeGray(gray);
  alphaSlider->changeAlpha(alpha);
  boxA->changeValue(alpha);
  boxY->changeValue(gray);
  boxV->changeValue(toValue(gray));
  changeColor();
}

void PickerImplGray::setValue(const int newValue) {
  gray = toGray(newValue);
  graySlider->changeGray(gray);
  alphaSlider->changeHSV({0, 0, newValue});
  boxY->changeValue(gray);
  changeColor();
}

void PickerImplGray::setGray(const int newGray) {
  gray = newGray;
  const int value = toValue(gray);
  boxV->changeValue(value);
  alphaSlider->changeHSV({0, 0, value});
  changeColor();
}

void PickerImplGray::setAlpha(const int newAlpha) {
  alpha = newAlpha;
  changeColor();
}

void PickerImplGray::changeColor() {
  Q_EMIT colorChanged(FormatYA::toPixel(gray, alpha));
}
