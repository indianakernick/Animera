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
#include <QtWidgets/qwidget.h>

class ComboBoxPopup;

class ComboBoxWidget final : public QWidget {
  Q_OBJECT
  
public:
  ComboBoxWidget(QWidget *, int);

  void clear();
  void addItem(const QString &);
  void setCurrentIndex(int);
  
  int count() const;
  QString itemText(int) const;
  int currentIndex() const;
  QString currentText() const;

  void showPopup();
  void hidePopup();

Q_SIGNALS:
  void currentIndexChanged(int);

private:
  TextIconRects rects;
  QPixmap arrow;
  ComboBoxPopup *popup = nullptr;
  std::vector<QString> items;
  size_t current = -1;

  void mousePressEvent(QMouseEvent *) override;
  void paintEvent(QPaintEvent *) override;
};

#endif
