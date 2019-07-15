//
//  icon push button widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 15/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef icon_push_button_widget_hpp
#define icon_push_button_widget_hpp

#include <QtWidgets/qabstractbutton.h>

class IconPushButtonWidget final : public QAbstractButton {
public:
  IconPushButtonWidget(QWidget *, QPixmap);

private:
  QPixmap pix;
  
  void paintEvent(QPaintEvent *) override;
};

#endif
