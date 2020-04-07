//
//  status bar widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 10/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_status_bar_widget_hpp
#define animera_status_bar_widget_hpp

#include <string_view>
#include <QtCore/qtimer.h>
#include <QtWidgets/qwidget.h>

class StatusBarWidget final : public QWidget {
  Q_OBJECT

public:
  explicit StatusBarWidget(QWidget *);

public Q_SLOTS:
  /// Show temporary text.
  /// Only shown for a short period of time before being automatically hidden.
  /// Covers normal and permanent text.
  void showTemp(std::string_view);
  /// Show normal text.
  /// Covers permanent text.
  void showNorm(std::string_view);
  /// Show permanent text.
  void showPerm(std::string_view);
  /// Show appending text.
  /// Appends the normal text for a short period of time before being
  /// automatically hidden.
  /// Separated from the normal text by a vertical line.
  void showApnd(std::string_view);
  
private:
  QString tempText;
  QString normText;
  QString permText;
  QString apndText;
  QTimer tempTimer;
  QTimer apndTimer;
  
  void paintEvent(QPaintEvent *) override;

private Q_SLOTS:
  void hideTemp();
  void hideApnd();
};

#endif
