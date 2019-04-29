//
//  color label widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef color_label_widget_hpp
#define color_label_widget_hpp

#include <QtWidgets/qwidget.h>

// @TODO maybe we could do something similar to text box
// accept a RectWidgetSize and an offsetX

class ColorLabelWidget final : public QWidget {
public:
  ColorLabelWidget(QWidget *, const QString &);

private:
  QString text;

  void paintEvent(QPaintEvent *) override;
};

#endif
