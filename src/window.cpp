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
#include "application.hpp"
#include "global font.hpp"
#include <QtWidgets/qstyle.h>
#include "separator widget.hpp"
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qfiledialog.h>

Window::Window(QWidget *parent, const QRect desktop)
  : QMainWindow{parent},
    bottom{this},
    right{this},
    editor{this},
    palette{&right},
    colors{&right},
    tools{this},
    timeline{&bottom},
    statusBar{&bottom},
    colorPicker{&right} {
  resize(glob_window_size);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setFocusPolicy(Qt::StrongFocus);
  setGeometry(QStyle::alignedRect(
    Qt::LeftToRight,
    Qt::AlignCenter,
    size(),
    desktop
  ));
  setupUI();
  setupMenubar();
  connectSignals();
}

bool Window::hasOpen(const QString &file) const {
  return fileName == file;
}

void Window::setupUI() {
  setStyleSheet("QMainWindow::separator {"
    "width: " + QString::number(glob_border_width) + "px;"
    "height: " + QString::number(glob_border_width) + "px;"
    "background-color: " + glob_border_color.name() + ";"
  "}"
  
  // @TODO remove this bug workaround
  // https://bugreports.qt.io/browse/QTBUG-75783
  "QMainWindow {"
    "background-color: " + glob_border_color.name() + ";"
  "}"
  
  // @TODO Maybe use the status bar instead of tooltips
  // This is too difficult to customize
  "QToolTip {"
    "background-color: " + glob_main.name() + ";"
    "color: " + glob_light_2.name() + ";"
    "font-family: \"" + getGlobalFont().family() + "\";"
    "font-size: " + QString::number(getGlobalFont().pointSize()) + "pt;"
    "border-width: " + QString::number(glob_border_width) + "px;"
    "border-color: " + glob_border_color.name() + ";"
    "border-style: solid;"
  "}"
  
  "QMenuBar {"
    "font-family: " + getGlobalFont().family() + ";"
    "font-size: " + QString::number(getGlobalFont().pointSize()) + "pt;"
    "background-color: " + glob_main.name() + ";"
    "color: " + glob_light_2.name() + ";"
  "}"
  
  "QMenu {"
    "background-color: " + glob_main.name() + ";"
  "}"
  
  "QMenu::separator {"
    "background-color: " + glob_light_2.name() + ";"
    "height: " + QString::number(glob_border_width) + "px;"
  "}"
  
  "QMenu::item {"
    "font-family: " + getGlobalFont().family() + ";"
    "font-size: " + QString::number(getGlobalFont().pointSize()) + "pt;"
    "background-color: " + glob_main.name() + ";"
    "color: " + glob_light_2.name() + ";"
  "}"
  
  "QMenu::item:selected {"
    "background-color: " + glob_light_1.name() + ";"
  "}"
  );
  
  QVBoxLayout *bottomLayout = new QVBoxLayout{&bottom};
  bottom.setLayout(bottomLayout);
  bottomLayout->setContentsMargins(0, 0, 0, 0);
  bottomLayout->setSpacing(0);
  bottomLayout->addWidget(&timeline);
  bottomLayout->addWidget(new HoriSeparator{&bottom});
  bottomLayout->addWidget(&statusBar);
  bottomLayout->setAlignment(Qt::AlignBottom);
  
  QVBoxLayout *rightLayout = new QVBoxLayout{&right};
  right.setLayout(rightLayout);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(glob_padding);
  rightLayout->addWidget(&colorPicker);
  rightLayout->addWidget(new HoriSeparator{&right});
  rightLayout->addWidget(&colors);
  rightLayout->addWidget(new HoriSeparator{&right});
  rightLayout->addWidget(&palette);
  rightLayout->addSpacing(glob_padding);
  right.setStyleSheet("background-color: " + glob_main.name());
  right.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  
  makeDockWidget(Qt::LeftDockWidgetArea, &tools);
  makeDockWidget(Qt::BottomDockWidgetArea, &bottom);
  makeDockWidget(Qt::RightDockWidgetArea, &right);
  setCentralWidget(&editor);
}

#define ADD_ACTION(MENU, NAME, SHORTCUT, WIDGET, MEMFN) do {                    \
  QAction *action = MENU->addAction(NAME);                                      \
  action->setShortcut(SHORTCUT);                                                \
  CONNECT(action, triggered, &WIDGET, MEMFN);                                   \
} while (0)

void Window::setupMenubar() {
  menubar = new QMenuBar{this};
  // menubar->setNativeMenuBar(false);
  if (!menubar->isNativeMenuBar()) {
    makeDockWidget(Qt::TopDockWidgetArea, menubar);
  }
  
  auto *app = static_cast<Application *>(QApplication::instance());
  QMenu *file = menubar->addMenu("File");
  ADD_ACTION(file, "New", QKeySequence::New, *app, newFileDialog);
  ADD_ACTION(file, "Open", QKeySequence::Open, *app, openFileDialog);
  ADD_ACTION(file, "Save", QKeySequence::Save, *this, saveFile);
  ADD_ACTION(file, "Save As", QKeySequence::SaveAs, *this, saveFileDialog);
  
  QMenu *layer = menubar->addMenu("Layer");
  ADD_ACTION(layer, "New Layer", Qt::SHIFT + Qt::Key_N, timeline, addLayer);
  ADD_ACTION(layer, "Delete Layer", Qt::SHIFT + Qt::Key_Backspace, timeline, removeLayer);
  ADD_ACTION(layer, "Move Layer Up", Qt::SHIFT + Qt::Key_Up, timeline, moveLayerUp);
  ADD_ACTION(layer, "Move Layer Down", Qt::SHIFT + Qt::Key_Down, timeline, moveLayerDown);
  ADD_ACTION(layer, "Toggle Visibility", Qt::SHIFT + Qt::Key_V, timeline, toggleLayerVisible);
  // @TODO Maybe consider this
  // We have to keep the action in sync with the timeline
  /*{
    QAction *action = layer->addAction("Visible");
    action->setShortcut(Qt::SHIFT + Qt::Key_V);
    action->setCheckable(true);
    action->setChecked(true);
    CONNECT(action, triggered, &timeline, toggleLayerVisible);
  }*/
  layer->addSeparator();
  ADD_ACTION(layer, "Layer Above", Qt::Key_W, timeline, layerAbove);
  ADD_ACTION(layer, "Layer Below", Qt::Key_S, timeline, layerBelow);
  
  QMenu *frame = menubar->addMenu("Frame");
  ADD_ACTION(frame, "New Frame", Qt::ALT + Qt::Key_N, timeline, addFrame);
  ADD_ACTION(frame, "New Empty Frame", Qt::ALT + Qt::Key_E, timeline, addNullFrame);
  ADD_ACTION(frame, "Delete Frame", Qt::ALT + Qt::Key_Backspace, timeline, removeFrame);
  frame->addSeparator();
  ADD_ACTION(frame, "Clear Frame", Qt::ALT + Qt::Key_C, timeline, clearFrame);
  ADD_ACTION(frame, "Extend Linked Frame", Qt::ALT + Qt::Key_L, timeline, extendFrame);
  frame->addSeparator();
  ADD_ACTION(frame, "Next Frame", Qt::Key_D, timeline, nextFrame);
  ADD_ACTION(frame, "Previous Frame", Qt::Key_A, timeline, prevFrame);
  ADD_ACTION(frame, "Play Animation", Qt::Key_Space, timeline, toggleAnimation);
}

#undef ADD_ACTION

void Window::makeDockWidget(Qt::DockWidgetArea area, QWidget *widget) {
  QDockWidget *dock = new QDockWidget{this};
  dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
  dock->setAllowedAreas(area);
  dock->setWidget(widget);
  dock->setTitleBarWidget(new QWidget{dock});
  addDockWidget(area, dock);
}

void Window::connectSignals() {
  CONNECT(&timeline, posChanged,      &tools,       changeCell);
  CONNECT(&timeline, posChanged,      &clear,       changePos);
  CONNECT(&timeline, posChanged,      &sample,      changePos);
  CONNECT(&timeline, posChanged,      &undo,        changePos);
  CONNECT(&timeline, visibleChanged,  &editor,      changeVisible);
  CONNECT(&timeline, frameChanged,    &editor,      changeFrame);
  CONNECT(&timeline, composite,       &editor,      composite);
  CONNECT(&timeline, canvasInitialized, &colors,    initCanvas);
  CONNECT(&timeline, canvasInitialized, &editor,    initCanvas);
  CONNECT(&timeline, canvasInitialized, &palette,   initCanvas);
  CONNECT(&timeline, canvasInitialized, &colorPicker, initCanvas);
  CONNECT(&timeline, canvasInitialized, &tools,     initCanvas);
  
  CONNECT(&tools,    cellModified,    &editor,      composite);
  CONNECT(&tools,    overlayModified, &editor,      compositeOverlay);
  CONNECT(&tools,    updateStatusBar, &statusBar,   showPerm);
  CONNECT(&tools,    cellRequested,   &timeline,    requestCell);
  
  CONNECT(&editor,   mouseLeave,      &tools,       mouseLeave);
  CONNECT(&editor,   mouseDown,       &tools,       mouseDown);
  CONNECT(&editor,   mouseMove,       &tools,       mouseMove);
  CONNECT(&editor,   mouseUp,         &tools,       mouseUp);
  CONNECT(&editor,   keyPress,        &tools,       keyPress);
  CONNECT(&editor,   keyPress,        &clear,       keyPress);
  CONNECT(&editor,   mouseMove,       &sample,      mouseMove);
  CONNECT(&editor,   keyPress,        &sample,      keyPress);
  CONNECT(&editor,   keyPress,        &undo,        keyPress);
  CONNECT(&tools,    changingAction,  &undo,        cellModified);
  
  CONNECT(&colors,   colorsChanged,   &tools,       changeColors);
  CONNECT(&colors,   attachColor,     &colorPicker, attach);
  CONNECT(&colors,   colorsChanged,   &clear,       changeColors);
  
  CONNECT(&clear,    cellModified,    &tools,       cellModified);
  
  CONNECT(&sample,   colorChanged,    &colorPicker, setColor);
  
  CONNECT(&undo,     cellReverted,    &editor,      composite);
  CONNECT(&undo,     showTempStatus,  &statusBar,   showTemp);
  
  CONNECT(&palette,  attachColor,     &colorPicker, attach);
  CONNECT(&palette,  setColor,        &colorPicker, setColor);
  CONNECT(&palette,  paletteChanged,  &timeline,    changePalette);
  CONNECT(&palette,  paletteChanged,  &editor,      changePalette);
  CONNECT(&palette,  paletteChanged,  &tools,       changePalette);
  CONNECT(&palette,  paletteColorChanged, &editor,  composite);
  
  CONNECT(this,      newFile,         this,         show);
  CONNECT(this,      newFile,         &colors,      initCanvas);
  CONNECT(this,      newFile,         &editor,      initCanvas);
  CONNECT(this,      newFile,         &palette,     initCanvas);
  CONNECT(this,      newFile,         &timeline,    initCanvas);
  CONNECT(this,      newFile,         &colorPicker, initCanvas);
  CONNECT(this,      newFile,         &tools,       initCanvas);
  
  CONNECT(this,      openFile,        this,         show);
  CONNECT(this,      openFile,        this,         setFileName);
  CONNECT(this,      openFile,        &timeline,    openFile);
}

void Window::setFileName(const QString &name) {
  fileName = name;
  setWindowTitle(name);
}

void Window::saveFile() {
  if (fileName.isEmpty()) {
    saveFileDialog();
  } else {
    timeline.saveFile(fileName);
    statusBar.showTemp("Saved!");
  }
}

void Window::saveFileDialog() {
  /*auto *dialog = new QFileDialog{this, "Save File"};
  CONNECT(dialog, fileSelected, this, setFileName);
  CONNECT(dialog, fileSelected, &timeline, saveFile);
  dialog->setNameFilter("Pixel 2 File (*.px2)");
  dialog->setDefaultSuffix("px2");
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->show();*/
  const QString saveFileName = QFileDialog::getSaveFileName(
    this,
    "Save File",
    QDir::homePath(),
    "Pixel 2 File (*.px2)"
  );
  if (!saveFileName.isEmpty()) {
    setFileName(saveFileName);
    timeline.saveFile(saveFileName);
  }
}

void Window::closeEvent(QCloseEvent *) {
  static_cast<Application *>(QApplication::instance())->windowClosed(this);
}

#include "window.moc"
