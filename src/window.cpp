//
//  window.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "window.hpp"

#include <QtWidgets/qstyle.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qdockwidget.h>

Window::Window(const QRect desktop)
  : bottom{this},
    editor{this, anim},
    tools{this},
    timeline{&bottom, anim},
    statusBar{&bottom} {
  setWindowTitle("Pixel 2");
  setMinimumSize(1280, 720);
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
  show();
  
  // @TODO remove
  anim.initialize(QSize{256, 256}, Format::color);
  anim.appendSource(0);
  timeline.projectLoaded();
}

void Window::setupUI() {
  bottom.setMinimumHeight(100);
  bottom.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
  QVBoxLayout *layout = new QVBoxLayout{&bottom};
  bottom.setLayout(layout);
  layout->addWidget(&timeline);
  layout->addWidget(&statusBar);
  layout->setContentsMargins(0, 0, 0, 0);
  bottom.setContentsMargins(0, 0, 0, 0);
  makeDockWidget(Qt::LeftDockWidgetArea, &tools);
  makeDockWidget(Qt::BottomDockWidgetArea, &bottom);
  setCentralWidget(&editor);
}

void Window::setupMenubar() {
  menubar = new QMenuBar{this};
  QMenu *file = menubar->addMenu("File");
  file->addAction("Open");
  file->addAction("Save");
  file->addSeparator();
  file->addAction("Export");
  // connect(open, &QAction::triggered, this, &Window::openDoc);
  // connect(save, &QAction::triggered, this, &Window::saveDoc);
}

void Window::makeDockWidget(Qt::DockWidgetArea area, QWidget *widget) {
  QDockWidget *dock = new QDockWidget{this};
  dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
  dock->setAllowedAreas(area);
  dock->setWidget(widget);
  dock->setTitleBarWidget(new QWidget{});
  addDockWidget(area, dock);
}

void Window::connectSignals() {
  connect(&timeline, &TimelineWidget::posChange,         &editor,    &EditorWidget::compositePos);
  connect(&timeline, &TimelineWidget::posChange,         &tools,     &ToolSelectWidget::changeCell);
  connect(&timeline, &TimelineWidget::layerVisibility,   &editor,    &EditorWidget::compositeVis);
  
  connect(&tools,    &ToolSelectWidget::cellModified,    &editor,    &EditorWidget::composite);
  connect(&tools,    &ToolSelectWidget::overlayModified, &editor,    &EditorWidget::compositeOverlay);
  connect(&tools,    &ToolSelectWidget::updateStatusBar, &statusBar, &StatusBarWidget::showPerm);
  
  connect(&editor,   &EditorWidget::mouseLeave,          &tools,     &ToolSelectWidget::mouseLeave);
  connect(&editor,   &EditorWidget::mouseDown,           &tools,     &ToolSelectWidget::mouseDown);
  connect(&editor,   &EditorWidget::mouseMove,           &tools,     &ToolSelectWidget::mouseMove);
  connect(&editor,   &EditorWidget::mouseUp,             &tools,     &ToolSelectWidget::mouseUp);
  connect(&editor,   &EditorWidget::keyPress,            &tools,     &ToolSelectWidget::keyPress);
}

#include "window.moc"
