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
    svGraph{new SVGraphWidget{this}},
    hueSlider{new HueSliderWidget{this}},
    alphaSlider{new AlphaSliderWidget{this}},
    boxR{new NumberInputWidget{this, 89, 255}},
    boxG{new NumberInputWidget{this, 89, 255}},
    boxB{new NumberInputWidget{this, 89, 255}},
    boxH{new NumberInputWidget{this, pick_default_color.hue(), 359}},
    boxS{new NumberInputWidget{this, 89, 100}},
    boxV{new NumberInputWidget{this, 89, 100}},
    boxA{new NumberInputWidget{this, pick_default_color.alpha(), 255}},
    boxHex{new HexInputWidget{this, 0x89898989}},
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
  
  // create an object that has a bunch of signals and slots for converting
  // between hsv and rgb
  
  // maybe it could be the ColorPickerWidget?
}

#include "color picker widget.moc"

