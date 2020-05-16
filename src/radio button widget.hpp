//
//  radio button widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 3/5/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_radio_button_widget_hpp
#define animera_radio_button_widget_hpp

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
