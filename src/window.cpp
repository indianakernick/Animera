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
#include <QtWidgets/qdockwidget.h>

Window::Window(const QRect desktop)
  : tools{this}, editor{this, anim}, timeline{this, anim} {
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
  anim.initialize(QSize{128, 128}, Format::color);
  anim.appendSource(0);
  timeline.projectLoaded();
}

void Window::setupUI() {
  makeDockWidget(Qt::LeftDockWidgetArea, &tools);
  makeDockWidget(Qt::BottomDockWidgetArea, &timeline);
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
  connect(&timeline, &TimelineWidget::posChange,       &editor, &EditorWidget::compositePos);
  connect(&timeline, &TimelineWidget::posChange,       &tools,  &ToolsWidget::changeCell);
  connect(&timeline, &TimelineWidget::layerVisibility, &editor, &EditorWidget::compositeVis);
  connect(&tools,    &ToolsWidget::cellModified,       &editor, &EditorWidget::composite);
  connect(&editor,   &EditorWidget::mouseDown,         &tools,  &ToolsWidget::mouseDown);
  connect(&editor,   &EditorWidget::mouseMove,         &tools,  &ToolsWidget::mouseMove);
  connect(&editor,   &EditorWidget::mouseUp,           &tools,  &ToolsWidget::mouseUp);
  connect(&editor,   &EditorWidget::keyPress,          &tools,  &ToolsWidget::keyPress);
}

#include "window.moc"
