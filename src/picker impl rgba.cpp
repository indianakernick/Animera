//
//  picker impl rgba.cpp
//  Animera
//
//  Created by Indi Kernick on 10/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "picker impl rgba.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "label widget.hpp"
#include <Graphics/format.hpp>
#include "color input widget.hpp"
#include "color graph widget.hpp"
#include "color slider widget.hpp"

void PickerImplRGBA::init(QWidget *parent) {
  colorHsv = color2hsv(pick_default_color);
  colorRgb = color2rgb(pick_default_color);
  alpha = pick_default_color.alpha();
  svGraph = new SVGraphWidget{parent};
  hueSlider = new HueSliderWidget{parent};
  alphaSlider = new AlphaSliderWidget{parent};
  boxR = new NumberInputWidget{parent, pick_number_rect, {0, 255, colorRgb.r}};
  boxG = new NumberInputWidget{parent, pick_number_rect, {0, 255, colorRgb.g}};
  boxB = new NumberInputWidget{parent, pick_number_rect, {0, 255, colorRgb.b}};
  boxH = new NumberInputWidget{parent, pick_number_rect, {0, 359, colorHsv.h}};
  boxS = new NumberInputWidget{parent, pick_number_rect, {0, 100, colorHsv.s}};
  boxV = new NumberInputWidget{parent, pick_number_rect, {0, 100, colorHsv.v}};
  boxA = new NumberInputWidget{parent, pick_number_rect, {0, 255, alpha}};
  boxHex = new HexInputWidget{parent, pick_hex_rect, colorRgb, alpha};
  labelR = new LabelWidget{parent, pick_label_rect, "R"};
  labelG = new LabelWidget{parent, pick_label_rect, "G"};
  labelB = new LabelWidget{parent, pick_label_rect, "B"};
  labelH = new LabelWidget{parent, pick_label_rect, "H"};
  labelS = new LabelWidget{parent, pick_label_rect, "S"};
  labelV = new LabelWidget{parent, pick_label_rect, "V"};
  labelA = new LabelWidget{parent, pick_label_rect, "A"};
  labelHex = new LabelWidget{parent, pick_label_rect, "#"};
}

void PickerImplRGBA::setupLayout(QGridLayout *layout) {
  layout->addWidget(svGraph,     1, 0, 1, 6);
  layout->addWidget(hueSlider,   2, 0, 1, 6);
  layout->addWidget(alphaSlider, 3, 0, 1, 6);
  
  layout->addWidget(labelR,      4, 0);
  layout->addWidget(boxR,        4, 1);
  layout->addWidget(labelG,      4, 2);
  layout->addWidget(boxG,        4, 3);
  layout->addWidget(labelB,      4, 4);
  layout->addWidget(boxB,        4, 5);
  
  layout->addWidget(labelH,      5, 0);
  layout->addWidget(boxH,        5, 1);
  layout->addWidget(labelS,      5, 2);
  layout->addWidget(boxS,        5, 3);
  layout->addWidget(labelV,      5, 4);
  layout->addWidget(boxV,        5, 5);
  
  layout->addWidget(labelA,      6, 0);
  layout->addWidget(boxA,        6, 1);
  layout->addWidget(labelHex,    6, 2);
  layout->addWidget(boxHex,      6, 3, 1, 3);
}

void PickerImplRGBA::connectSignals() {
  CONNECT(svGraph,     svChanged,    hueSlider,   setSV);
  CONNECT(hueSlider,   hueChanged,   svGraph,     setHue);
  CONNECT(boxH,        valueChanged, svGraph,     setHue);
  CONNECT(hueSlider,   hueChanged,   alphaSlider, setHue);
  CONNECT(svGraph,     svChanged,    alphaSlider, setSV);

  CONNECT(hueSlider,   hueChanged,   alphaSlider, setHue);
  CONNECT(svGraph,     svChanged,    alphaSlider, setSV);
  
  CONNECT(alphaSlider, alphaChanged, boxA,        setValue);
  CONNECT(boxA,        valueChanged, alphaSlider, setAlpha);
  
  CONNECT(hueSlider,   hueChanged,   boxH,        setValue);
  CONNECT(boxH,        valueChanged, hueSlider,   setHue);
  
  CONNECT(hueSlider,   hueChanged,   this,        setHue);
  CONNECT(boxH,        valueChanged, this,        setHue);
  
  CONNECT(svGraph,     svChanged,    this,        setSVfromGraph);
  CONNECT(boxS,        valueChanged, this,        setSVfromBoxS);
  CONNECT(boxV,        valueChanged, this,        setSVfromBoxV);
  
  CONNECT(alphaSlider, alphaChanged, this,        setAlpha);
  CONNECT(boxA,        valueChanged, this,        setAlpha);
  
  CONNECT(boxR,        valueChanged, this,        setRed);
  CONNECT(boxG,        valueChanged, this,        setGreen);
  CONNECT(boxB,        valueChanged, this,        setBlue);
  
  CONNECT(boxHex,      rgbaChanged,  this,        setRGBA);
}

void PickerImplRGBA::setColor(const QRgb color) {
  colorRgb.r = qRed(color);
  colorRgb.g = qGreen(color);
  colorRgb.b = qBlue(color);
  alpha = qAlpha(color);
  setRGBA(colorRgb, alpha);
  boxHex->setRgba(colorRgb, alpha);
}

void PickerImplRGBA::setSVfromGraph(const int sat, const int val) {
  colorHsv.s = sat;
  colorHsv.v = val;
  Q_EMIT hueSlider->setSV(sat, val);
  Q_EMIT alphaSlider->setSV(sat, val);
  Q_EMIT boxS->setValue(sat);
  Q_EMIT boxV->setValue(val);
  changeRGB();
}

void PickerImplRGBA::setSVfromBoxS(const int sat) {
  colorHsv.s = sat;
  Q_EMIT hueSlider->setSV(sat, colorHsv.v);
  Q_EMIT alphaSlider->setSV(sat, colorHsv.v);
  Q_EMIT svGraph->setSV(sat, colorHsv.v);
  changeRGB();
}

void PickerImplRGBA::setSVfromBoxV(const int val) {
  colorHsv.v = val;
  Q_EMIT hueSlider->setSV(colorHsv.s, val);
  Q_EMIT alphaSlider->setSV(colorHsv.s, val);
  Q_EMIT svGraph->setSV(colorHsv.s, val);
  changeRGB();
}

void PickerImplRGBA::setAlpha(const int alp) {
  alpha = alp;
  Q_EMIT boxHex->setRgba(colorRgb, alp);
  changeColor();
}

void PickerImplRGBA::setHue(const int hue) {
  colorHsv.h = hue;
  changeRGB();
}

void PickerImplRGBA::setRGBA(const RGB rgb, const int alp) {
  colorRgb = rgb;
  alpha = alp;
  changeHSV();
  Q_EMIT alphaSlider->setAlpha(alp);
  Q_EMIT boxA->setValue(alp);
  Q_EMIT boxR->setValue(rgb.r);
  Q_EMIT boxG->setValue(rgb.g);
  Q_EMIT boxB->setValue(rgb.b);
  changeColor();
}

void PickerImplRGBA::setRed(const int red) {
  colorRgb.r = red;
  changeHSV();
  Q_EMIT boxHex->setRgba(colorRgb, alpha);
  changeColor();
}

void PickerImplRGBA::setGreen(const int green) {
  colorRgb.g = green;
  changeHSV();
  Q_EMIT boxHex->setRgba(colorRgb, alpha);
  changeColor();
}

void PickerImplRGBA::setBlue(const int blue) {
  colorRgb.b = blue;
  changeHSV();
  Q_EMIT boxHex->setRgba(colorRgb, alpha);
  changeColor();
}

void PickerImplRGBA::changeRGB() {
  colorRgb = hsv2rgb(colorHsv);
  Q_EMIT boxR->setValue(colorRgb.r);
  Q_EMIT boxG->setValue(colorRgb.g);
  Q_EMIT boxB->setValue(colorRgb.b);
  Q_EMIT boxHex->setRgba(colorRgb, alpha);
  changeColor();
}

void PickerImplRGBA::changeHSV() {
  colorHsv = rgb2hsv(colorRgb);
  Q_EMIT boxH->setValue(colorHsv.h);
  Q_EMIT boxS->setValue(colorHsv.s);
  Q_EMIT boxV->setValue(colorHsv.v);
  Q_EMIT hueSlider->setHSV(colorHsv);
  Q_EMIT alphaSlider->setHSV(colorHsv);
  Q_EMIT svGraph->setHSV(colorHsv);
}

void PickerImplRGBA::changeColor() {
  Q_EMIT colorChanged(gfx::ARGB::pixel(colorRgb.r, colorRgb.g, colorRgb.b, alpha));
}
