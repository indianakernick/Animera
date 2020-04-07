//
//  picker impl rgba.hpp
//  Animera
//
//  Created by Indiana Kernick on 10/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef picker_impl_rgba_hpp
#define picker_impl_rgba_hpp

#include "color convert.hpp"
#include "color picker impl.hpp"

class SVGraphWidget;
class HueSliderWidget;
class AlphaSliderWidget;
class NumberInputWidget;
class HexInputWidget;
class LabelWidget;

class PickerImplRGBA final : public ColorPickerImpl {
public:
  void init(QWidget *) override;
  void setupLayout(QGridLayout *) override;
  void connectSignals() override;
  void setColor(QRgb) override;

private Q_SLOTS:
  void setSVfromGraph(int, int);
  void setSVfromBoxS(int);
  void setSVfromBoxV(int);
  void setAlpha(int);
  void setHue(int);
  void setRGBA(RGB, int);
  void setRed(int);
  void setGreen(int);
  void setBlue(int);

private:
  void changeRGB();
  void changeHSV();
  void changeColor();

  HSV colorHsv;
  RGB colorRgb;
  int alpha;

  SVGraphWidget *svGraph = nullptr;
  HueSliderWidget *hueSlider = nullptr;
  AlphaSliderWidget *alphaSlider = nullptr;
  
  NumberInputWidget *boxR = nullptr;
  NumberInputWidget *boxG = nullptr;
  NumberInputWidget *boxB = nullptr;
  NumberInputWidget *boxH = nullptr;
  NumberInputWidget *boxS = nullptr;
  NumberInputWidget *boxV = nullptr;
  NumberInputWidget *boxA = nullptr;
  HexInputWidget *boxHex = nullptr;
  
  LabelWidget *labelR = nullptr;
  LabelWidget *labelG = nullptr;
  LabelWidget *labelB = nullptr;
  LabelWidget *labelH = nullptr;
  LabelWidget *labelS = nullptr;
  LabelWidget *labelV = nullptr;
  LabelWidget *labelA = nullptr;
  LabelWidget *labelHex = nullptr;
};

#endif
