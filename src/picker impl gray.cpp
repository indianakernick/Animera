//
//  picker impl gray.cpp
//  Animera
//
//  Created by Indiana Kernick on 10/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "picker impl gray.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "label widget.hpp"
#include <Graphics/format.hpp>
#include "color input widget.hpp"
#include "color slider widget.hpp"

namespace {

constexpr int toValue(const int gray) {
  return scale(gray, 255, 100);
}

constexpr int toGray(const int value) {
  return scale(value, 100, 255);
}

}

void PickerImplGray::init(QWidget *parent) {
  gray = pick_default_gray;
  alpha = pick_default_color.alpha();
  graySlider = new GraySliderWidget{parent, gray, alpha};
  alphaSlider = new AlphaSliderWidget{parent, {gray, gray, gray}, alpha, true};
  boxA = new NumberInputWidget{parent, pick_number_rect, {0, 255, alpha}};
  boxY = new NumberInputWidget{parent, pick_number_rect, {0, 255, gray}};
  boxV = new NumberInputWidget{parent, pick_number_rect, {0, 100, toValue(gray)}};
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
  CONNECT(graySlider,  grayChanged,    boxY,        setValue);
  CONNECT(boxY,        valueChanged,   graySlider,  setGray);

  CONNECT(graySlider,  grayChanged,    this,        setGray);
  CONNECT(boxY,        valueChanged,   this,        setGray);
  
  CONNECT(alphaSlider, alphaChanged,   boxA,        setValue);
  CONNECT(boxA,        valueChanged,   alphaSlider, setAlpha);
  
  CONNECT(alphaSlider, alphaChanged,   this,        setAlpha);
  CONNECT(boxA,        valueChanged,   this,        setAlpha);
  
  CONNECT(boxV,        valueChanged,   this,        setValue);
  
  CONNECT(graySlider,  shouldShowNorm, this,        shouldShowNorm);
  CONNECT(alphaSlider, shouldShowNorm, this,        shouldShowNorm);
}

void PickerImplGray::setColor(const QRgb color) {
  gray = gfx::YA::gray(color);
  alpha = gfx::YA::alpha(color);
  graySlider->setGray(gray);
  alphaSlider->setRgba({gray, gray, gray}, alpha);
  boxA->setValue(alpha);
  boxY->setValue(gray);
  boxV->setValue(toValue(gray));
  changeColor();
}

void PickerImplGray::setValue(const int newValue) {
  gray = toGray(newValue);
  graySlider->setGray(gray);
  alphaSlider->setRgba({gray, gray, gray}, alpha);
  boxY->setValue(gray);
  changeColor();
}

void PickerImplGray::setGray(const int newGray) {
  gray = newGray;
  boxV->setValue(toValue(gray));
  alphaSlider->setRgba({gray, gray, gray}, alpha);
  changeColor();
}

void PickerImplGray::setAlpha(const int newAlpha) {
  alpha = newAlpha;
  graySlider->setAlpha(newAlpha);
  changeColor();
}

void PickerImplGray::changeColor() {
  Q_EMIT colorChanged(gfx::YA::pixel(gray, alpha));
}
