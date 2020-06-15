//
//  combo box widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 20/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_combo_box_widget_hpp
#define animera_combo_box_widget_hpp

#include "config geometry.hpp"
#include <QtWidgets/qwidget.h>

class ComboBoxPopup;

/*
QComboBox has some wierd positioning issues
It seems to shift itself by a few pixels
https://forum.qt.io/topic/105191/why-isn-t-a-qcombobox-positioned-correctly-in-a-layout
*/

class ComboBoxWidget final : public QWidget {
  Q_OBJECT
  
public:
  enum class OrderPolicy {
    constant, // selected item doesn't move
    bottom,   // selected item moves to bottom
    top       // selected item moves to top
  };

  ComboBoxWidget(QWidget *, int);

  void setPolicy(OrderPolicy);
  void clearWithItem(const QString &);
  void addItem(const QString &);
  void setCurrentIndex(int);
  
  int count() const;
  QString itemText(int) const;
  int currentIndex() const;
  QString currentText() const;

Q_SIGNALS:
  void currentIndexChanged(int);
  void currentTextChanged(const QString &);

private:
  TextIconRects rects;
  QRect outer;
  QPixmap arrow;
  ComboBoxPopup *popup = nullptr;
  std::vector<QString> items;
  int current = -1;
  OrderPolicy policy = OrderPolicy::constant;

  void enforcePolicy();

  void mousePressEvent(QMouseEvent *) override;
  void paintEvent(QPaintEvent *) override;
};

#endif
