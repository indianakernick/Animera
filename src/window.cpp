//
//  window.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "window.hpp"

#include "config.hpp"
#include "connect.hpp"
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qdockwidget.h>

Window::Window(const QRect desktop)
  : bottom{this},
    right{this},
    editor{this, anim},
    colors{&right},
    tools{this},
    timeline{&bottom, anim},
    statusBar{&bottom},
    colorPicker{&right} {
  setWindowTitle("Pixel 2");
  setMinimumSize(glob_min_window_size);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setGeometry(QStyle::alignedRect(
    Qt::LeftToRight,
    Qt::AlignCenter,
    size(),
    desktop
  ));
  setupUI();
  setupMenubar();
  connectSignals();
  colors.attachPrimary();
  show();
  
  // @TODO remove
  anim.initialize(QSize{256, 256}, Format::color);
  anim.appendSource(0);
  timeline.projectLoaded();
}

void Window::setupUI() {
  bottom.setMinimumHeight(100);
  bottom.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
  
  QVBoxLayout *bottomLayout = new QVBoxLayout{&bottom};
  bottom.setLayout(bottomLayout);
  bottom.setContentsMargins(0, 0, 0, 0);
  bottomLayout->setContentsMargins(0, 0, 0, 0);
  bottomLayout->setSpacing(0);
  bottomLayout->addWidget(&timeline);
  bottomLayout->addWidget(&statusBar);
  bottomLayout->setAlignment(Qt::AlignBottom);
  
  QVBoxLayout *rightLayout = new QVBoxLayout{&right};
  right.setLayout(rightLayout);
  right.setContentsMargins(0, 0, 0, 0);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(0);
  rightLayout->addWidget(&colorPicker);
  rightLayout->addWidget(&colors);
  rightLayout->setAlignment(Qt::AlignTop);
  right.setStyleSheet("background-color: " + glob_main.name());
  right.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
  
  makeDockWidget(Qt::LeftDockWidgetArea, &tools);
  makeDockWidget(Qt::BottomDockWidgetArea, &bottom);
  makeDockWidget(Qt::RightDockWidgetArea, &right);
  setCentralWidget(&editor);
}

void Window::setupMenubar() {
  menubar = new QMenuBar{this};
  QMenu *file = menubar->addMenu("File");
  file->addAction("Open");
  file->addAction("Save");
  file->addSeparator();
  file->addAction("Export");
  // CONNECT(open, triggered, this, openDoc);
  // CONNECT(save, triggered, this, saveDoc);
}

void Window::makeDockWidget(Qt::DockWidgetArea area, QWidget *widget) {
  QDockWidget *dock = new QDockWidget{this};
  dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
  dock->setAllowedAreas(area);
  dock->setWidget(widget);
  dock->setTitleBarWidget(new QWidget{dock});
  addDockWidget(area, dock);
}

void Window::connectSignals() {
  CONNECT(&timeline, posChange,       &editor,      compositePos);
  CONNECT(&timeline, posChange,       &tools,       changeCell);
  CONNECT(&timeline, posChange,       &clear,       posChange);
  CONNECT(&timeline, posChange,       &undo,        posChange);
  CONNECT(&timeline, layerVisibility, &editor,      compositeVis);
  
  CONNECT(&tools,    cellModified,    &undo,        cellModified);
  CONNECT(&tools,    cellModified,    &editor,      composite);
  CONNECT(&tools,    overlayModified, &editor,      compositeOverlay);
  CONNECT(&tools,    updateStatusBar, &statusBar,   showPerm);
  
  CONNECT(&editor,   mouseLeave,      &tools,       mouseLeave);
  CONNECT(&editor,   mouseDown,       &tools,       mouseDown);
  CONNECT(&editor,   mouseMove,       &tools,       mouseMove);
  CONNECT(&editor,   mouseUp,         &tools,       mouseUp);
  CONNECT(&editor,   keyPress,        &tools,       keyPress);
  CONNECT(&editor,   keyPress,        &clear,       keyPress);
  CONNECT(&editor,   keyPress,        &undo,        keyPress);
  
  CONNECT(&colors,   colorsChanged,   &tools,       changeColors);
  CONNECT(&colors,   attachColor,     &colorPicker, attach);
  
  CONNECT(&clear,    cellModified,    &tools,       cellModified);
  
  CONNECT(&undo,     cellReverted,    &editor,      composite);
  CONNECT(&undo,     showTempStatus,  &statusBar,   showTemp);
  
}

#include "window.moc"
