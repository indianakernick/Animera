//
//  icon radio button widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef icon_radio_button_widget_hpp
#define icon_radio_button_widget_hpp

#include "radio button widget.hpp"

class IconRadioButtonWidget : public RadioButtonWidget {
public:
  IconRadioButtonWidget(QWidget *, QPixmap, QPixmap);

private:
  QPixmap onPix;
  QPixmap offPix;

  void paintEvent(QPaintEvent *) override;
};

#endif
