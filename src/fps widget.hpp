//
//  fps widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 6/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef fps_widget_hpp
#define fps_widget_hpp

#include <QtWidgets/qwidget.h>
#include <QtCore/qelapsedtimer.h>

class FPSWidget final : public QWidget {
public:
  explicit FPSWidget(QWidget *);

  void start();

private:
  QElapsedTimer timer;
  int frames = 0;

  void paintEvent(QPaintEvent *) override;
};

#endif
