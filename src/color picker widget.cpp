//
//  color picker widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color picker widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "color input widget.hpp"
#include "color label widget.hpp"
#include "color graph widget.hpp"
#include "color slider widget.hpp"
#include <QtWidgets/qgridlayout.h>

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
  : QWidget{parent},
    colorHsv{color2hsv(pick_default_color)},
    colorRgb{color2rgb(pick_default_color)},
    alpha{pick_default_color.alpha()},
    svGraph{new SVGraphWidget{this}},
    hueSlider{new HueSliderWidget{this}},
    alphaSlider{new AlphaSliderWidget{this}},
    boxR{new NumberInputWidget{this, colorRgb.r, 255}},
    boxG{new NumberInputWidget{this, colorRgb.g, 255}},
    boxB{new NumberInputWidget{this, colorRgb.b, 255}},
    boxH{new NumberInputWidget{this, colorHsv.h, 359}},
    boxS{new NumberInputWidget{this, colorHsv.s, 100}},
    boxV{new NumberInputWidget{this, colorHsv.v, 100}},
    boxA{new NumberInputWidget{this, alpha, 255}},
    boxHex{new HexInputWidget{this, colorRgb, alpha}},
    labelR{new LabelWidget{this, "R", pick_label_rect}},
    labelG{new LabelWidget{this, "G", pick_label_rect}},
    labelB{new LabelWidget{this, "B", pick_label_rect}},
    labelH{new LabelWidget{this, "H", pick_label_rect}},
    labelS{new LabelWidget{this, "S", pick_label_rect}},
    labelV{new LabelWidget{this, "V", pick_label_rect}},
    labelA{new LabelWidget{this, "A", pick_label_rect}},
    labelHex{new LabelWidget{this, "#", pick_label_rect}} {
  setFixedWidth(pick_svgraph_rect.widget().width());
  setupLayout();
  connectSignals();
  show();
}

void ColorPickerWidget::setupLayout() {
  QGridLayout *layout = new QGridLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  
  layout->addWidget(svGraph, 0, 0, 1, 6);
  layout->addWidget(hueSlider, 1, 0, 1, 6);
  layout->addWidget(alphaSlider, 2, 0, 1, 6);
  
  layout->addWidget(labelR, 3, 0);
  layout->addWidget(boxR,   3, 1);
  layout->addWidget(labelG, 3, 2);
  layout->addWidget(boxG,   3, 3);
  layout->addWidget(labelB, 3, 4);
  layout->addWidget(boxB,   3, 5);
  
  layout->addWidget(labelH, 4, 0);
  layout->addWidget(boxH,   4, 1);
  layout->addWidget(labelS, 4, 2);
  layout->addWidget(boxS,   4, 3);
  layout->addWidget(labelV, 4, 4);
  layout->addWidget(boxV,   4, 5);
  
  layout->addWidget(labelA,   5, 0);
  layout->addWidget(boxA,     5, 1);
  layout->addWidget(labelHex, 5, 2);
  layout->addWidget(boxHex,   5, 3, 1, 3);
  
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);
}

void ColorPickerWidget::connectSignals() {
  CONNECT(svGraph,     svChanged,    hueSlider,   changeSV);
  CONNECT(hueSlider,   hueChanged,   svGraph,     changeHue);
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

void ColorPickerWidget::changeRGB() {
  colorRgb = hsv2rgb(colorHsv);
  Q_EMIT boxR->changeValue(colorRgb.r);
  Q_EMIT boxG->changeValue(colorRgb.g);
  Q_EMIT boxB->changeValue(colorRgb.b);
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
}

void ColorPickerWidget::changeHSV() {
  colorHsv = rgb2hsv(colorRgb);
  Q_EMIT boxH->changeValue(colorHsv.h);
  Q_EMIT boxS->changeValue(colorHsv.s);
  Q_EMIT boxV->changeValue(colorHsv.v);
  Q_EMIT hueSlider->changeHSV(colorHsv);
  Q_EMIT alphaSlider->changeHSV(colorHsv);
  Q_EMIT svGraph->changeHSV(colorHsv);
}

void ColorPickerWidget::changeSVfromGraph(const int sat, const int val) {
  colorHsv.s = sat;
  colorHsv.v = val;
  Q_EMIT hueSlider->changeSV(sat, val);
  Q_EMIT alphaSlider->changeSV(sat, val);
  Q_EMIT boxS->changeValue(sat);
  Q_EMIT boxV->changeValue(val);
  changeRGB();
}

void ColorPickerWidget::changeSVfromBoxS(const int sat) {
  colorHsv.s = sat;
  Q_EMIT hueSlider->changeSV(sat, colorHsv.v);
  Q_EMIT alphaSlider->changeSV(sat, colorHsv.v);
  Q_EMIT svGraph->changeSV(sat, colorHsv.v);
  changeRGB();
}

void ColorPickerWidget::changeSVfromBoxV(const int val) {
  colorHsv.v = val;
  Q_EMIT hueSlider->changeSV(colorHsv.s, val);
  Q_EMIT alphaSlider->changeSV(colorHsv.s, val);
  Q_EMIT svGraph->changeSV(colorHsv.s, val);
  changeRGB();
}

void ColorPickerWidget::changeAlpha(const int alp) {
  alpha = alp;
  Q_EMIT boxHex->changeRgba(colorRgb, alp);
}

void ColorPickerWidget::changeHue(const int hue) {
  colorHsv.h = hue;
  changeRGB();
}

void ColorPickerWidget::changeRGBA(const RGB rgb, const int alp) {
  colorRgb = rgb;
  alpha = alp;
  changeHSV();
  Q_EMIT alphaSlider->alphaChanged(alp);
  Q_EMIT boxA->valueChanged(alp);
}

void ColorPickerWidget::changeRed(const int red) {
  colorRgb.r = red;
  changeHSV();
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
}

void ColorPickerWidget::changeGreen(const int green) {
  colorRgb.g = green;
  changeHSV();
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
}

void ColorPickerWidget::changeBlue(const int blue) {
  colorRgb.b = blue;
  changeHSV();
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
}

#include "color picker widget.moc"

