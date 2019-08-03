//
//  picker impl gray.hpp
//  Animera
//
//  Created by Indi Kernick on 10/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef picker_impl_gray_hpp
#define picker_impl_gray_hpp

#include "color picker impl.hpp"

class GraySliderWidget;
class AlphaSliderWidget;
class NumberInputWidget;
class HexInputWidget;
class LabelWidget;

class PickerImplGray final : public ColorPickerImpl {
public:
  void init(QWidget *) override;
  void setupLayout(QGridLayout *) override;
  void connectSignals() override;
  void setColor(QRgb) override;

private Q_SLOTS:
  void setValue(int);
  void setGray(int);
  void setAlpha(int);

private:
  int gray;
  int alpha;
  
  GraySliderWidget *graySlider = nullptr;
  AlphaSliderWidget *alphaSlider = nullptr;
  NumberInputWidget *boxA = nullptr;
  NumberInputWidget *boxY = nullptr;
  NumberInputWidget *boxV = nullptr;
  LabelWidget *labelA = nullptr;
  LabelWidget *labelY = nullptr;
  LabelWidget *labelV = nullptr;
  
  void changeColor();
};

#endif
