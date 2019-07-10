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
#include "label widget.hpp"
#include "color input widget.hpp"
#include "color graph widget.hpp"
#include "color slider widget.hpp"
#include <QtWidgets/qgridlayout.h>

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
  : QWidget{parent} {}

void ColorPickerWidget::initCanvas(const Format newFormat) {
  format = newFormat;
  if (format == Format::rgba || format == Format::palette) {
    colorHsv = color2hsv(pick_default_color);
    colorRgb = color2rgb(pick_default_color);
    alpha = pick_default_color.alpha();
    nameLabel = new LabelWidget{this, pick_name_rect, "PALETTE 42"};
    svGraph = new SVGraphWidget{this};
    hueSlider = new HueSliderWidget{this};
    alphaSlider = new AlphaSliderWidget{this};
    boxR = new NumberInputWidget{this, pick_number_rect, colorRgb.r, 255};
    boxG = new NumberInputWidget{this, pick_number_rect, colorRgb.g, 255};
    boxB = new NumberInputWidget{this, pick_number_rect, colorRgb.b, 255};
    boxH = new NumberInputWidget{this, pick_number_rect, colorHsv.h, 359};
    boxS = new NumberInputWidget{this, pick_number_rect, colorHsv.s, 100};
    boxV = new NumberInputWidget{this, pick_number_rect, colorHsv.v, 100};
    boxA = new NumberInputWidget{this, pick_number_rect, alpha, 255};
    boxHex = new HexInputWidget{this, pick_hex_rect, colorRgb, alpha};
    labelR = new LabelWidget{this, pick_label_rect, "R"};
    labelG = new LabelWidget{this, pick_label_rect, "G"};
    labelB = new LabelWidget{this, pick_label_rect, "B"};
    labelH = new LabelWidget{this, pick_label_rect, "H"};
    labelS = new LabelWidget{this, pick_label_rect, "S"};
    labelV = new LabelWidget{this, pick_label_rect, "V"};
    labelA = new LabelWidget{this, pick_label_rect, "A"};
    labelHex = new LabelWidget{this, pick_label_rect, "#"};
  } else if (format == Format::gray) {
    gray = pick_default_gray;
    colorRgb = {gray, gray, gray};
    alpha = pick_default_color.alpha();
    nameLabel = new LabelWidget{this, pick_name_rect, "PALETTE 42"};
    graySlider = new GraySliderWidget{this};
    boxY = new NumberInputWidget{this, pick_number_rect, gray, 255};
    boxHex = new HexInputWidget{this, pick_hex_rect, colorRgb, alpha};
    boxHex->setReadOnly(true);
    labelY = new LabelWidget{this, pick_label_rect, "Y"};
    labelHex = new LabelWidget{this, pick_label_rect, "#"};
  } else Q_UNREACHABLE();
  setFixedWidth(pick_svgraph_rect.widget().width() + 2 * glob_padding);
  setupLayout();
  connectSignals();
  show();
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

void ColorPickerWidget::setupLayout() {
  QGridLayout *layout = new QGridLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(glob_padding, glob_padding, glob_padding, glob_padding);
  
  if (format == Format::rgba || format == Format::palette) {
    layout->addWidget(nameLabel, 0, 0, 1, 6);
    layout->addWidget(svGraph, 1, 0, 1, 6);
    layout->addWidget(hueSlider, 2, 0, 1, 6);
    layout->addWidget(alphaSlider, 3, 0, 1, 6);
    
    layout->addWidget(labelR, 4, 0);
    layout->addWidget(boxR,   4, 1);
    layout->addWidget(labelG, 4, 2);
    layout->addWidget(boxG,   4, 3);
    layout->addWidget(labelB, 4, 4);
    layout->addWidget(boxB,   4, 5);
    
    layout->addWidget(labelH, 5, 0);
    layout->addWidget(boxH,   5, 1);
    layout->addWidget(labelS, 5, 2);
    layout->addWidget(boxS,   5, 3);
    layout->addWidget(labelV, 5, 4);
    layout->addWidget(boxV,   5, 5);
    
    layout->addWidget(labelA,   6, 0);
    layout->addWidget(boxA,     6, 1);
    layout->addWidget(labelHex, 6, 2);
    layout->addWidget(boxHex,   6, 3, 1, 3);
  } else if (format == Format::gray) {
    layout->addWidget(nameLabel,  0, 0, 1, 6);
    layout->addWidget(graySlider, 1, 0, 1, 6);
    layout->addWidget(labelY,     2, 0);
    layout->addWidget(boxY,       2, 1);
    layout->addWidget(labelHex,   2, 2);
    layout->addWidget(boxHex,     2, 3, 1, 3);
  } else Q_UNREACHABLE();
  
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);
}

// @TODO there has to be a better way

void ColorPickerWidget::connectSignals() {
  if (format == Format::rgba || format == Format::palette) {
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
  } else if (format == Format::gray) {
    CONNECT(graySlider,  grayChanged,  boxY,        changeValue);
    CONNECT(boxY,        valueChanged, alphaSlider, changeAlpha);
    
    CONNECT(graySlider,  grayChanged,  this,        changeGray);
    CONNECT(boxY,        valueChanged, this,        changeGray);
    
    //CONNECT(boxHex,      rgbaChanged,  this,        changeRGBAtoGray);
  } else Q_UNREACHABLE();
}

void ColorPickerWidget::updateHandle() {
  if (handle) {
    handle->changeColor(qRgba(colorRgb.r, colorRgb.g, colorRgb.b, alpha));
  }
}

void ColorPickerWidget::setColor(const QRgb color) {
  if (format == Format::rgba || format == Format::palette) {
    colorRgb.r = qRed(color);
    colorRgb.g = qGreen(color);
    colorRgb.b = qBlue(color);
    alpha = qAlpha(color);
    changeRGBA(colorRgb, alpha);
    boxHex->changeRgba(colorRgb, alpha);
  } else if (format == Format::gray) {
    gray = color & 255;
    colorRgb = {gray, gray, gray};
    alpha = 255;
    graySlider->changeGray(gray);
    boxY->changeValue(gray);
    boxHex->changeRgba(colorRgb, alpha);
    //changeRGBAtoGray(colorRgb);
  } else Q_UNREACHABLE();
}

void ColorPickerWidget::changeRGB() {
  colorRgb = hsv2rgb(colorHsv);
  Q_EMIT boxR->changeValue(colorRgb.r);
  Q_EMIT boxG->changeValue(colorRgb.g);
  Q_EMIT boxB->changeValue(colorRgb.b);
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
  updateHandle();
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
  updateHandle();
}

void ColorPickerWidget::changeHue(const int hue) {
  colorHsv.h = hue;
  changeRGB();
}

void ColorPickerWidget::changeRGBA(const RGB rgb, const int alp) {
  colorRgb = rgb;
  alpha = alp;
  changeHSV();
  Q_EMIT alphaSlider->changeAlpha(alp);
  Q_EMIT boxA->changeValue(alp);
  Q_EMIT boxR->changeValue(rgb.r);
  Q_EMIT boxG->changeValue(rgb.g);
  Q_EMIT boxB->changeValue(rgb.b);
  updateHandle();
}

void ColorPickerWidget::changeRed(const int red) {
  colorRgb.r = red;
  changeHSV();
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
  updateHandle();
}

void ColorPickerWidget::changeGreen(const int green) {
  colorRgb.g = green;
  changeHSV();
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
  updateHandle();
}

void ColorPickerWidget::changeBlue(const int blue) {
  colorRgb.b = blue;
  changeHSV();
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
  updateHandle();
}

void ColorPickerWidget::changeGray(const int newGray) {
  gray = newGray;
  colorRgb = {gray, gray, gray};
  Q_EMIT boxHex->changeRgba(colorRgb, alpha);
  updateHandle();
}

#include "color picker widget.moc"
