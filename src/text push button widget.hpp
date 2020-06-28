//
//  text push button widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 20/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_text_push_button_widget_hpp
#define animera_text_push_button_widget_hpp

#include "widget rect.hpp"
#include <QtWidgets/qabstractbutton.h>

class TextPushButtonWidget : public QAbstractButton {
public:
  TextPushButtonWidget(QWidget *, const WidgetRect &, const QString &);

private:
  WidgetRect rect;
  QPixmap pixmap;

  void paintEvent(QPaintEvent *) override;
};

#endif
