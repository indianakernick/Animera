//
//  combo box widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef combo_box_widget_hpp
#define combo_box_widget_hpp

#include "config.hpp"
#include <QtWidgets/qcombobox.h>

using Base = QComboBox;

class ComboBoxWidget final : public Base {
public:
  ComboBoxWidget(QWidget *, WidgetRect);

private:
  WidgetRect rect;
  QPixmap arrow;

  void paintEvent(QPaintEvent *) override;
};

#endif
