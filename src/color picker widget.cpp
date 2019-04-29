//
//  color picker widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color picker widget.hpp"

#include "config.hpp"
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
    labelR{new ColorLabelWidget{this, "R"}},
    labelG{new ColorLabelWidget{this, "G"}},
    labelB{new ColorLabelWidget{this, "B"}},
    labelH{new ColorLabelWidget{this, "H"}},
    labelS{new ColorLabelWidget{this, "S"}},
    labelV{new ColorLabelWidget{this, "V"}},
    labelA{new ColorLabelWidget{this, "A"}},
    labelHex{new ColorLabelWidget{this, "#"}} {
  setFixedWidth(pick_svgraph_size.widget().width());
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

// @TODO yes or no?
#define CONNECT(SENDER, SENDER_FUN, RECEIVER, RECEIVER_FUN)                     \
  QObject::connect(                                                             \
    SENDER,                                                                     \
    &std::remove_cv_t<std::remove_pointer_t<decltype(SENDER)>>::SENDER_FUN,                                \
    RECEIVER,                                                                   \
    &std::remove_cv_t<std::remove_pointer_t<decltype(RECEIVER)>>::RECEIVER_FUN                             \
  )

void ColorPickerWidget::connectSignals() {
  CONNECT(svGraph, svChanged, hueSlider, changeSV);

  //connect(svGraph, &SVGraphWidget::svChanged, hueSlider, &HueSliderWidget::changeSV);
  connect(hueSlider, &HueSliderWidget::hueChanged, svGraph, &SVGraphWidget::changeHue);
  connect(hueSlider, &HueSliderWidget::hueChanged, alphaSlider, &AlphaSliderWidget::changeHue);
  connect(svGraph, &SVGraphWidget::svChanged, alphaSlider, &AlphaSliderWidget::changeSV);
  
  connect(alphaSlider, &AlphaSliderWidget::alphaChanged, boxA, &NumberInputWidget::changeValue);
  connect(boxA, &NumberInputWidget::valueChanged, alphaSlider, &AlphaSliderWidget::changeAlpha);
  
  connect(hueSlider, &HueSliderWidget::hueChanged, boxH, &NumberInputWidget::changeValue);
  connect(boxH, &NumberInputWidget::valueChanged, hueSlider, &HueSliderWidget::changeHue);
  
  // create an object that has a bunch of signals and slots for converting
  // between hsv and rgb
  
  // maybe it could be the ColorPickerWidget?
}

#include "color picker widget.moc"

