//
//  scroll bar widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 21/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_scroll_bar_widget_hpp
#define animera_scroll_bar_widget_hpp

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
  int rightMargin() const;
  int bottomMargin() const;

private:
  ScrollCornerWidget *corner;
};

#endif
