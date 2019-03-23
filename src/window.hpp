//
//  window.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef window_hpp
#define window_hpp

#include "tools widget.hpp"
#include "editor widget.hpp"
#include "timeline widget.hpp"
#include "status bar widget.hpp"
#include <QtWidgets/qmainwindow.h>

class Window final : public QMainWindow {
  Q_OBJECT

public:
  explicit Window(QRect);
  
private:
  QWidget bottom;
  Animation anim;
  ToolsWidget tools;
  EditorWidget editor;
  TimelineWidget timeline;
  StatusBarWidget statusBar;
  QMenuBar *menubar = nullptr;
  
  void setupUI();
  void setupMenubar();
  void makeDockWidget(Qt::DockWidgetArea, QWidget *);
  void connectSignals();
};

#endif
