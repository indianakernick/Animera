//
//  color label widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef color_label_widget_hpp
#define color_label_widget_hpp

#include "config.hpp"
#include <QtWidgets/qwidget.h>

class LabelWidget final : public QWidget {
public:
  LabelWidget(QWidget *, const QString &, WidgetRect);

private:
  QString text;
  WidgetRect rect;

  void paintEvent(QPaintEvent *) override;
};

#endif
