//
//  scroll bar widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 21/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef scroll_bar_widget_hpp
#define scroll_bar_widget_hpp

#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qscrollarea.h>

class ScrollBarWidget final : public QScrollBar {
public:
  ScrollBarWidget(Qt::Orientation, QWidget *);
  
private:
  void paintEvent(QPaintEvent *) override;
};

class ScrollCornerWidget final : public QWidget {
public:
  explicit ScrollCornerWidget(QWidget *);
  
private:
  void paintEvent(QPaintEvent *) override;
};

class ScrollAreaWidget : public QScrollArea {
public:
  explicit ScrollAreaWidget(QWidget *);

protected:
  void adjustMargins();
};

#endif
