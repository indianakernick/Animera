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

void Window::newFile(const Format format, const QSize size) {
  show();
  sprite.newFile(format, size);
}

void Window::openFile(const QString &path) {
  show();
  setFileName(path);
  sprite.openFile(path);
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
  ADD_ACTION(layer, "New Layer", Qt::SHIFT + Qt::Key_N, sprite.timeline, insertLayer);
  ADD_ACTION(layer, "Delete Layer", Qt::SHIFT + Qt::Key_Backspace, sprite.timeline, removeLayer);
  ADD_ACTION(layer, "Move Layer Up", Qt::SHIFT + Qt::Key_Up, sprite.timeline, moveLayerUp);
  ADD_ACTION(layer, "Move Layer Down", Qt::SHIFT + Qt::Key_Down, sprite.timeline, moveLayerDown);
  //ADD_ACTION(layer, "Toggle Visibility", Qt::SHIFT + Qt::Key_V, sprite.timeline, toggleLayerVisible);
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
  ADD_ACTION(layer, "Layer Above", Qt::Key_W, sprite.timeline, layerAbove);
  ADD_ACTION(layer, "Layer Below", Qt::Key_S, sprite.timeline, layerBelow);
  
  QMenu *frame = menubar->addMenu("Frame");
  ADD_ACTION(frame, "New Frame", Qt::ALT + Qt::Key_N, sprite.timeline, insertFrame);
  ADD_ACTION(frame, "New Empty Frame", Qt::ALT + Qt::Key_E, sprite.timeline, insertNullFrame);
  ADD_ACTION(frame, "Delete Frame", Qt::ALT + Qt::Key_Backspace, sprite.timeline, removeFrame);
  frame->addSeparator();
  ADD_ACTION(frame, "Clear Cell", Qt::ALT + Qt::Key_C, sprite.timeline, clearCell);
  ADD_ACTION(frame, "Extend Linked Cell", Qt::ALT + Qt::Key_L, sprite.timeline, extendCell);
  frame->addSeparator();
  ADD_ACTION(frame, "Next Frame", Qt::Key_D, sprite.timeline, nextFrame);
  ADD_ACTION(frame, "Previous Frame", Qt::Key_A, sprite.timeline, prevFrame);
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
  CONNECT(sprite.timeline, currCellChanged,     tools,           setCell);
  CONNECT(sprite.timeline, currCellChanged,     clear,           setCell);
  CONNECT(sprite.timeline, currCellChanged,     sample,          setCell);
  CONNECT(sprite.timeline, currCellChanged,     undo,            setCell);
  CONNECT(sprite.timeline, frameChanged,        editor,          setFrame);
  CONNECT(sprite.timeline, currPosChanged,      timeline,        setCurrPos);
  CONNECT(sprite.timeline, visibilityChanged,   timeline,        setVisibility);
  CONNECT(sprite.timeline, nameChanged,         timeline,        setName);
  CONNECT(sprite.timeline, layerChanged,        timeline,        setLayer);
  CONNECT(sprite.timeline, frameCountChanged,   timeline,        setFrameCount);
  CONNECT(sprite.timeline, layerCountChanged,   timeline,        setLayerCount);
  
  CONNECT(timeline,        visibilityChanged,   sprite.timeline, setVisibility);
  CONNECT(timeline,        nameChanged,         sprite.timeline, setName);
  CONNECT(timeline,        nextFrame,           sprite.timeline, nextFrame);
  
  CONNECT(sprite,          canvasInitialized,   colorPicker,     initCanvas);
  CONNECT(sprite,          canvasInitialized,   colors,          initCanvas);
  CONNECT(sprite,          canvasInitialized,   editor,          initCanvas);
  CONNECT(sprite,          canvasInitialized,   palette,         initCanvas);
  CONNECT(sprite,          canvasInitialized,   tools,           initCanvas);
  
  CONNECT(sprite.palette,  paletteChanged,      palette,         setPalette);
  CONNECT(sprite.palette,  paletteChanged,      editor,          setPalette);
  CONNECT(sprite.palette,  paletteChanged,      tools,           setPalette);
  CONNECT(sprite.palette,  paletteChanged,      colors,          setPalette);
  
  CONNECT(tools,           cellModified,        editor,          composite);
  CONNECT(tools,           overlayModified,     editor,          compositeOverlay);
  CONNECT(tools,           shouldShowPerm,      statusBar,       showPerm);
  CONNECT(tools,           cellRequested,       sprite.timeline, requestCell);
  
  CONNECT(editor,          mouseLeave,          tools,           mouseLeave);
  CONNECT(editor,          mouseDown,           tools,           mouseDown);
  CONNECT(editor,          mouseMove,           tools,           mouseMove);
  CONNECT(editor,          mouseUp,             tools,           mouseUp);
  CONNECT(editor,          keyPress,            tools,           keyPress);
  CONNECT(editor,          keyPress,            clear,           keyPress);
  CONNECT(editor,          mouseMove,           sample,          mouseMove);
  CONNECT(editor,          keyPress,            sample,          keyPress);
  CONNECT(editor,          keyPress,            undo,            keyPress);
  CONNECT(tools,           changingAction,      undo,            cellModified);
  
  CONNECT(colors,          colorsChanged,       tools,           setColors);
  CONNECT(colors,          colorsChanged,       clear,           setColors);
  CONNECT(colors,          shouldAttachColor,   colorPicker,     attach);
  CONNECT(colors,          shouldAttachIndex,   palette,         attachIndex);
  
  CONNECT(clear,           cellModified,        tools,           cellModified);
  
  CONNECT(sample,          colorChanged,        colorPicker,     setColor);
  
  CONNECT(undo,            cellReverted,        editor,          composite);
  CONNECT(undo,            shouldShowTemp,      statusBar,       showTemp);
  
  CONNECT(palette,         shouldAttachColor,   colorPicker,     attach);
  CONNECT(palette,         shouldSetColor,      colorPicker,     setColor);
  CONNECT(palette,         shouldSetIndex,      colors,          setIndex);
  CONNECT(palette,         paletteColorChanged, editor,          composite);
  CONNECT(palette,         paletteColorChanged, colors,          changePaletteColors);
}

void Window::setFileName(const QString &name) {
  fileName = name;
  setWindowTitle(name);
}

void Window::saveFile() {
  if (fileName.isEmpty()) {
    saveFileDialog();
  } else {
    sprite.saveFile(fileName);
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
    sprite.saveFile(saveFileName);
  }
}

void Window::closeEvent(QCloseEvent *) {
  static_cast<Application *>(QApplication::instance())->windowClosed(this);
}

#include "window.moc"
