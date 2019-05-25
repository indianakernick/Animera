//
//  separator widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 25/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef separator_widget_hpp
#define separator_widget_hpp

#include <QtWidgets/qwidget.h>

class HoriSeparator : public QWidget {
public:
  explicit HoriSeparator(QWidget *);
  
private:
  void paintEvent(QPaintEvent *) override;
};

class VertSeparator : public QWidget {
public:
  explicit VertSeparator(QWidget *);
  
private:
  void paintEvent(QPaintEvent *) override;
};

#endif
