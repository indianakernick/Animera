//
//  separator widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 25/5/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_separator_widget_hpp
#define animera_separator_widget_hpp

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
