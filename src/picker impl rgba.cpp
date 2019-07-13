//
//  picker impl rgba.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "picker impl rgba.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "label widget.hpp"
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
  boxR = new NumberInputWidget{parent, pick_number_rect, colorRgb.r, 255};
  boxG = new NumberInputWidget{parent, pick_number_rect, colorRgb.g, 255};
  boxB = new NumberInputWidget{parent, pick_number_rect, colorRgb.b, 255};
  boxH = new NumberInputWidget{parent, pick_number_rect, colorHsv.h, 359};
  boxS = new NumberInputWidget{parent, pick_number_rect, colorHsv.s, 100};
  boxV = new NumberInputWidget{parent, pick_number_rect, colorHsv.v, 100};
  boxA = new NumberInputWidget{parent, pick_number_rect, alpha, 255};
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
  // @TODO there has to be a better way
  
  CONNECT(svGraph,     svChanged,    hueSlider,   changeSV);
  CONNECT(hueSlider,   hueChanged,   svGraph,     changeHue);
  CONNECT(boxH,        valueChanged, svGraph,     changeHue);
  CONNECT(hueSlider,   hueChanged,   alphaSlider, changeHue);
  CONNECT(svGraph,     svChanged,    alphaSlider, changeSV);

  CONNECT(hueSlider,   hueChanged,   alphaSlider, changeHue);
  CONNECT(svGraph,     svChanged,    alphaSlider, changeSV);
  
  CONNECT(alphaSlider, alphaChanged, boxA,        changeValue);
  CONNECT(boxA,        valueChanged, alphaSlider, changeAlpha);
  
  CONNECT(hueSlider,   hueChanged,   boxH,        changeValue);
  CONNECT(boxH,        valueChanged, hueSlider,   changeHue);
  
  CONNECT(hueSlider,   hueChanged,   this,        changeHue);
  CONNECT(boxH,        valueChanged, this,        changeHue);
  
  CONNECT(svGraph,     svChanged,    this,        changeSVfromGraph);
  CONNECT(boxS,        valueChanged, this,        changeSVfromBoxS);
  CONNECT(boxV,        valueChanged, this,        changeSVfromBoxV);
  
  CONNECT(alphaSlider, alphaChanged, this,        changeAlpha);
  CONNECT(boxA,        valueChanged, this,        changeAlpha);
  
  CONNECT(boxR,        valueChanged, this,        changeRed);
  CONNECT(boxG,        valueChanged, this,        changeGreen);
  CONNECT(boxB,        valueChanged, this,        changeBlue);
  
  CONNECT(boxHex,      rgbaChanged,  this,        changeRGBA);
}

void PickerImplRGBA::setColor(const QRgb color) {
  colorRgb.r = qRed(color);
  colorRgb.g = qGreen(color);
  colorRgb.b = qBlue(color);
  alpha = qAlpha(color);
  changeRGBA(colorRgb, alpha);
  boxHex->changeRgba(colorRgb, alpha);
}

void PickerImplRGBA::changeSVfromGraph(const int sat, const int val) {
  colorHsv.s = sat;
  colorHsv.v = val;
  Q_EMIT hueSlider->changeSV(sat, val);
  Q_EMIT alphaSlider->changeSV(sat, val);
  Q_EMIT boxS->changeValue(sat);
  Q_EMIT boxV->changeValue(val);
  changeRGB();
}

void PickerImplRGBA::changeSVfromBoxS(const int sat) {
  colorHsv.s = sat;
  Q_EMIT hueSlider->changeSV(sat, colorHsv.v);
  Q_EMIT alphaSlider->changeSV(sat, colorHsv.v);
  Q_EMIT svGraph->changeSV(sat, colorHsv.v);
  changeRGB();
}

void PickerImplRGBA::changeSVfromBoxV(const int val) {
  colorHsv.v = val;
  Q_EMIT hueSlider->changeSV(colorHsv.s, val);
  Q_EMIT alphaSlider->changeSV(colorHsv.s, val);
  Q_EMIT svGraph->changeSV(colorHsv.s, val);
  changeRGB();
}

void PickerImplRGBA::changeAlpha(const int alp) {
  alpha = alp;
  Q_EMIT boxHex->changeRgba(colorRgb, alp);
  changeColor();
}

void PickerImplRGBA::changeHue(const int hue) {
  colorHsv.h = hue;
  changeRGB();
}

void PickerImplRGBA::changeRGBA(const RGB rgb, const int alp) {
  colorRgb = rgb;
  alpha = alp;
  changeHSV();
  Q_EMIT alphaSlider->changeAlpha(alp);
  Q_EMIT boxA->changeValue(alp);
  Q_EMIT boxR->changeValue(rgb.r);
  Q_EMIT boxG->changeValue(rgb.g);
  Q_EMIT boxB->changeValue(rgb.b);
  changeColor();
}

void PickerImplRGBA::changeRed(const int red) {
  colorRgb.r = red;
  changeHSV();
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
  changeColor();
}

void PickerImplRGBA::changeGreen(const int green) {
  colorRgb.g = green;
  changeHSV();
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
  changeColor();
}

void PickerImplRGBA::changeBlue(const int blue) {
  colorRgb.b = blue;
  changeHSV();
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
  changeColor();
}

void PickerImplRGBA::changeRGB() {
  colorRgb = hsv2rgb(colorHsv);
  Q_EMIT boxR->changeValue(colorRgb.r);
  Q_EMIT boxG->changeValue(colorRgb.g);
  Q_EMIT boxB->changeValue(colorRgb.b);
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
  changeColor();
}

void PickerImplRGBA::changeHSV() {
  colorHsv = rgb2hsv(colorRgb);
  Q_EMIT boxH->changeValue(colorHsv.h);
  Q_EMIT boxS->changeValue(colorHsv.s);
  Q_EMIT boxV->changeValue(colorHsv.v);
  Q_EMIT hueSlider->changeHSV(colorHsv);
  Q_EMIT alphaSlider->changeHSV(colorHsv);
  Q_EMIT svGraph->changeHSV(colorHsv);
}

void PickerImplRGBA::changeColor() {
  Q_EMIT colorChanged(qRgba(colorRgb.r, colorRgb.g, colorRgb.b, alpha));
}
