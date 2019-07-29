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

class ComboBoxPopup;

class ComboBoxWidget final : public QComboBox {
public:
  ComboBoxWidget(QWidget *, int);

  void showPopup() override;
  void hidePopup() override;

private:
  TextIconRects rects;
  QPixmap arrow;
  ComboBoxPopup *popup = nullptr;

  void paintEvent(QPaintEvent *) override;
};

#endif
