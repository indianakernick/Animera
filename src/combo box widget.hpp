//
//  combo box widget.hpp
//  Animera
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef combo_box_widget_hpp
#define combo_box_widget_hpp

#include "config.hpp"
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
  ComboBoxWidget(QWidget *, int);

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

  void mousePressEvent(QMouseEvent *) override;
  void paintEvent(QPaintEvent *) override;
};

#endif
