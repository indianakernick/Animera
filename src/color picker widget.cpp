//
//  color picker widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "color picker widget.hpp"

class SVGraph final : public QWidget {
public:
  explicit SVGraph(QWidget *parent)
    : QWidget{parent} {}
};

class HueSlider final : public QWidget {
public:
  explicit HueSlider(QWidget *parent)
    : QWidget{parent} {}
};

class AlphaSlider final : public QWidget {
public:
  explicit AlphaSlider(QWidget *parent)
    : QWidget{parent} {}
};

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
  : QWidget{parent},
    svGraph{new SVGraph{this}},
    hueSlider{new HueSlider{this}},
    alphaSlider{new AlphaSlider{this}} {
  setMinimumSize(100, 100);
}

#include "color picker widget.moc"
