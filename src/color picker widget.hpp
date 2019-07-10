//
//  color picker widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef color_picker_widget_hpp
#define color_picker_widget_hpp

#include "image.hpp"
#include "color handle.hpp"
#include "color convert.hpp"
#include <QtWidgets/qwidget.h>

class SVGraphWidget;
class HueSliderWidget;
class AlphaSliderWidget;
class GraySliderWidget;
class NumberInputWidget;
class HexInputWidget;
class LabelWidget;

class ColorPickerWidget final : public QWidget {
  Q_OBJECT

public:
  explicit ColorPickerWidget(QWidget *);

public Q_SLOTS:
  void initCanvas(Format);
  void attach(ColorHandle *);
  void setColor(QRgb);
  
private:
  Format format;
  ColorHandle *handle = nullptr;
  HSV colorHsv;
  RGB colorRgb;
  int alpha;
  int gray;
  
  LabelWidget *nameLabel = nullptr;
  
  SVGraphWidget *svGraph = nullptr;
  HueSliderWidget *hueSlider = nullptr;
  AlphaSliderWidget *alphaSlider = nullptr;
  GraySliderWidget *graySlider = nullptr;
  
  NumberInputWidget *boxR = nullptr;
  NumberInputWidget *boxG = nullptr;
  NumberInputWidget *boxB = nullptr;
  NumberInputWidget *boxH = nullptr;
  NumberInputWidget *boxS = nullptr;
  NumberInputWidget *boxV = nullptr;
  NumberInputWidget *boxA = nullptr;
  NumberInputWidget *boxY = nullptr;
  HexInputWidget *boxHex = nullptr;
  
  LabelWidget *labelR = nullptr;
  LabelWidget *labelG = nullptr;
  LabelWidget *labelB = nullptr;
  LabelWidget *labelH = nullptr;
  LabelWidget *labelS = nullptr;
  LabelWidget *labelV = nullptr;
  LabelWidget *labelA = nullptr;
  LabelWidget *labelY = nullptr;
  LabelWidget *labelHex = nullptr;
  
  void setupLayout();
  void connectSignals();

  void updateHandle();

  void changeRGB();
  void changeHSV();

private Q_SLOTS:
  void changeSVfromGraph(int, int);
  void changeSVfromBoxS(int);
  void changeSVfromBoxV(int);
  void changeAlpha(int);
  void changeHue(int);
  void changeRGBA(RGB, int);
  void changeRed(int);
  void changeGreen(int);
  void changeBlue(int);
  void changeGray(int);
  //void changeRGBAtoGray(RGB);
};

#endif
