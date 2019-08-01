//
//  radio button widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef radio_button_widget_hpp
#define radio_button_widget_hpp

#include <QtWidgets/qabstractbutton.h>

/*
Radio buttons don't seem to work properly
Clicking them only sometimes checks them
The solution is to use mousePressEvent instead of mouseReleaseEvent
*/

class RadioButtonWidget : public QAbstractButton {
public:
  explicit RadioButtonWidget(QWidget *);

  void uncheck();

protected:
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
};

#endif
