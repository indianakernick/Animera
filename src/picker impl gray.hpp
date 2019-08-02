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
  void changeGray(int);

private:
  int gray;
  
  GraySliderWidget *graySlider = nullptr;
  NumberInputWidget *boxY = nullptr;
  HexInputWidget *boxHex = nullptr;
  LabelWidget *labelY = nullptr;
  LabelWidget *labelHex = nullptr;
};

#endif
