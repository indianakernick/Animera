//
//  label widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef label_widget_hpp
#define label_widget_hpp

#include "config.hpp"
#include <QtWidgets/qwidget.h>

class LabelWidget final : public QWidget {
public:
  LabelWidget(QWidget *, WidgetRect, const QString &);

  void setText(const QString &);

private:
  QString text;
  WidgetRect rect;

  void paintEvent(QPaintEvent *) override;
};

#endif
