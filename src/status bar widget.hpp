//
//  status bar widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef status_bar_widget_hpp
#define status_bar_widget_hpp

#include <string>
#include <QtCore/qtimer.h>
#include <QtWidgets/qlabel.h>

class StatusBarWidget final : public QWidget {
  Q_OBJECT

public:
  explicit StatusBarWidget(QWidget *);

public Q_SLOTS:
  void showTemp(std::string_view);
  void showPerm(std::string_view);
  
private:
  std::string permText;
  std::string tempText;
  QTimer timer;
  
  void paintEvent(QPaintEvent *) override;

private Q_SLOTS:
  void hideTemp();
};

#endif
