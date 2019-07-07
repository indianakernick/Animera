//
//  timeline controls widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_controls_widget_hpp
#define timeline_controls_widget_hpp

#include <QtCore/qtimer.h>
#include <QtWidgets/qwidget.h>

class ControlsWidget final : public QWidget {
  Q_OBJECT

public:
  explicit ControlsWidget(QWidget *);

Q_SIGNALS:
  void nextFrame();

public Q_SLOTS:
  void toggleAnimation();

private:
  QTimer animTimer;
  
  void paintEvent(QPaintEvent *) override;
};

#endif
