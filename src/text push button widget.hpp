//
//  text push button widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef text_push_button_widget_hpp
#define text_push_button_widget_hpp

#include "config.hpp"
#include <QtWidgets/qabstractbutton.h>

class TextPushButtonWidget final : public QAbstractButton {
public:
  TextPushButtonWidget(QWidget *, WidgetRect, const QString &);

private:
  WidgetRect rect;
  QString text;

  void paintEvent(QPaintEvent *) override;
};

#endif
