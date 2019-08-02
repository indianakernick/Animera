//
//  window.cpp
//  Animera
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "window.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "application.hpp"
#include "global font.hpp"
#include "export dialog.hpp"
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qtooltip.h>
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

void Window::newFile(const Format format, const QSize size) {
  show();
  sprite.newFile(format, size);
  setWindowModified(true);
}

void Window::openFile(const QString &path) {
  show();
  sprite.openFile(path);
  setWindowFilePath(path);
  setWindowModified(false);
}

void Window::modify() {
  setWindowModified(true);
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
  
  "QToolTip {"
    "background-color: " + glob_main.name() + ";"
    "color: " + glob_light_2.name() + ";"
    "border-width: " + QString::number(glob_border_width) + "px;"
    "border-color: " + glob_border_color.name() + ";"
    "border-style: solid;"
  "}"
  
  "QMenuBar {"
    "background-color: " + glob_main.name() + ";"
    "color: " + glob_light_2.name() + ";"
    "outline-style: none;"
  "}"
  
  "QMenuBar::item:open {"
    "background-color: " + glob_light_1.name() + ";"
  "}"
  
  "QMenu {"
    "background-color: " + glob_main.name() + ";"
    "padding: 0;"
    "outline-style: none;"
    "border-width: " + QString::number(glob_border_width) + "px;"
    "border-color: " + glob_border_color.name() + ";"
    "border-style: solid;"
  "}"
  
  "QMenu::separator {"
    "background-color: " + glob_light_2.name() + ";"
    "height: " + QString::number(glob_border_width) + "px;"
    "margin-top: " + QString::number(glob_margin) + "px;"
    "margin-bottom: " + QString::number(glob_margin) + "px;"
  "}"
  
  "QMenu::item {"
    "background-color: " + glob_main.name() + ";"
    "color: " + glob_light_2.name() + ";"
    "padding: " + QString::number(glob_text_margin) + "px;"
  "}"
  
  "QMenu::item:selected {"
    "background-color: " + glob_light_1.name() + ";"
  "}"
  );
  QToolTip::setFont(getGlobalFont());
  
  auto *bottomLayout = new QVBoxLayout{&bottom};
  bottomLayout->setContentsMargins(0, 0, 0, 0);
  bottomLayout->setSpacing(0);
  bottomLayout->addWidget(&timeline);
  bottomLayout->addWidget(new HoriSeparator{&bottom});
  bottomLayout->addWidget(&statusBar);
  bottomLayout->setAlignment(Qt::AlignBottom);
  
  auto *rightLayout = new QVBoxLayout{&right};
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(glob_margin);
  rightLayout->addWidget(&colorPicker);
  rightLayout->addWidget(new HoriSeparator{&right});
  rightLayout->addWidget(&colors);
  rightLayout->addWidget(new HoriSeparator{&right});
  rightLayout->addWidget(&palette);
  rightLayout->addSpacing(glob_margin);
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
  menubar->setNativeMenuBar(false);
  // if (!menubar->isNativeMenuBar()) {
    makeDockWidget(Qt::TopDockWidgetArea, menubar);
  // }
  menubar->setFont(getGlobalFont());
  
  auto *app = static_cast<Application *>(QApplication::instance());
  QMenu *file = menubar->addMenu("File");
  file->setFont(getGlobalFont());
  ADD_ACTION(file, "New", QKeySequence::New, *app, newFileDialog);
  ADD_ACTION(file, "Open", QKeySequence::Open, *app, openFileDialog);
  ADD_ACTION(file, "Save", QKeySequence::Save, *this, saveFile);
  ADD_ACTION(file, "Save As", QKeySequence::SaveAs, *this, saveFileDialog);
  ADD_ACTION(file, "Export", Qt::CTRL + Qt::Key_E, *this, exportDialog);
  
  QMenu *layer = menubar->addMenu("Layer");
  layer->setFont(getGlobalFont());
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
  frame->setFont(getGlobalFont());
  ADD_ACTION(frame, "New Frame", Qt::ALT + Qt::Key_N, sprite.timeline, insertFrame);
  ADD_ACTION(frame, "New Empty Frame", Qt::ALT + Qt::Key_E, sprite.timeline, insertNullFrame);
  ADD_ACTION(frame, "Delete Frame", Qt::ALT + Qt::Key_Backspace, sprite.timeline, removeFrame);
  frame->addSeparator();
  ADD_ACTION(frame, "Clear Cell", Qt::ALT + Qt::Key_C, sprite.timeline, clearCell);
  ADD_ACTION(frame, "Extend Linked Cell", Qt::ALT + Qt::Key_L, sprite.timeline, extendCell);
  ADD_ACTION(frame, "Split Linked Cell", Qt::ALT + Qt::Key_S, sprite.timeline, splitCell);
  frame->addSeparator();
  ADD_ACTION(frame, "Next Frame", Qt::Key_D, sprite.timeline, nextFrame);
  ADD_ACTION(frame, "Previous Frame", Qt::Key_A, sprite.timeline, prevFrame);
  ADD_ACTION(frame, "Play Animation", Qt::Key_Space, timeline, toggleAnimation);
  
  QMenu *selection = menubar->addMenu("Selection");
  selection->setFont(getGlobalFont());
  ADD_ACTION(selection, "Clear", Qt::CTRL + Qt::Key_X, sprite.timeline, clearSelected);
  ADD_ACTION(selection, "Copy", Qt::CTRL + Qt::Key_C, sprite.timeline, copySelected);
  ADD_ACTION(selection, "Paste", Qt::CTRL + Qt::Key_V, sprite.timeline, pasteSelected);
}

#undef ADD_ACTION

void Window::makeDockWidget(Qt::DockWidgetArea area, QWidget *widget) {
  auto *dock = new QDockWidget{this};
  dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
  dock->setAllowedAreas(area);
  // @TODO report bug and remove workaround
  auto *wrapper = new QWidget{widget->parentWidget()};
  // @TODO docks shouldn't be resizable
  auto *layout = new QVBoxLayout{wrapper};
  layout->setContentsMargins(0, 1, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(widget);
  dock->setWidget(wrapper);
  dock->setTitleBarWidget(new QWidget{dock});
  addDockWidget(area, dock);
}

void Window::connectSignals() {
  // @TODO Connections to the modify slot are scattered around.
  // I'd rather just CONNECT(sprite, modified, this, modify)

  CONNECT(sprite.timeline, currCellChanged,     tools,           setCell);
  CONNECT(sprite.timeline, currCellChanged,     clear,           setCell);
  CONNECT(sprite.timeline, currCellChanged,     sample,          setCell);
  CONNECT(sprite.timeline, currCellChanged,     undo,            setCell);
  CONNECT(sprite.timeline, frameChanged,        editor,          setFrame);
  CONNECT(sprite.timeline, currPosChanged,      timeline,        setCurrPos);
  CONNECT(sprite.timeline, selectionChanged,    timeline,        setSelection);
  CONNECT(sprite.timeline, visibilityChanged,   timeline,        setVisibility);
  CONNECT(sprite.timeline, nameChanged,         timeline,        setName);
  CONNECT(sprite.timeline, layerChanged,        timeline,        setLayer);
  CONNECT(sprite.timeline, frameCountChanged,   timeline,        setFrameCount);
  CONNECT(sprite.timeline, layerCountChanged,   timeline,        setLayerCount);
  CONNECT(sprite.timeline, modified,            this,            modify);
  
  CONNECT(timeline,        visibilityChanged,   sprite.timeline, setVisibility);
  CONNECT(timeline,        nameChanged,         sprite.timeline, setName);
  CONNECT(timeline,        nextFrame,           sprite.timeline, nextFrame);
  CONNECT(timeline,        insertLayer,         sprite.timeline, insertLayer);
  CONNECT(timeline,        removeLayer,         sprite.timeline, removeLayer);
  CONNECT(timeline,        moveLayerUp,         sprite.timeline, moveLayerUp);
  CONNECT(timeline,        moveLayerDown,       sprite.timeline, moveLayerDown);
  CONNECT(timeline,        extendCell,          sprite.timeline, extendCell);
  CONNECT(timeline,        splitCell,           sprite.timeline, splitCell);
  CONNECT(timeline,        beginSelection,      sprite.timeline, beginSelection);
  CONNECT(timeline,        continueSelection,   sprite.timeline, continueSelection);
  CONNECT(timeline,        endSelection,        sprite.timeline, endSelection);
  CONNECT(timeline,        clearSelection,      sprite.timeline, clearSelection);
  CONNECT(timeline,        currPosChanged,      sprite.timeline, setCurrPos);
  
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
  CONNECT(tools,           changingAction,      undo,            cellModified);
  CONNECT(tools,           changingAction,      this,            modify);
  
  CONNECT(editor,          overlayChanged,      tools,           setOverlay);
  CONNECT(editor,          mouseLeave,          tools,           mouseLeave);
  CONNECT(editor,          mouseDown,           tools,           mouseDown);
  CONNECT(editor,          mouseMove,           tools,           mouseMove);
  CONNECT(editor,          mouseUp,             tools,           mouseUp);
  CONNECT(editor,          keyPress,            tools,           keyPress);
  CONNECT(editor,          keyPress,            clear,           keyPress);
  CONNECT(editor,          mouseMove,           sample,          mouseMove);
  CONNECT(editor,          keyPress,            sample,          keyPress);
  CONNECT(editor,          keyPress,            undo,            keyPress);
  
  CONNECT(colors,          colorsChanged,       tools,           setColors);
  CONNECT(colors,          colorsChanged,       clear,           setColors);
  CONNECT(colors,          shouldAttachColor,   colorPicker,     attach);
  CONNECT(colors,          shouldAttachIndex,   palette,         attachIndex);
  
  CONNECT(clear,           cellModified,        tools,           cellModified);
  CONNECT(clear,           cellModified,        this,            modify);
  
  CONNECT(sample,          colorChanged,        colorPicker,     setColor);
  
  CONNECT(undo,            cellReverted,        editor,          composite);
  CONNECT(undo,            cellReverted,        this,            modify);
  CONNECT(undo,            shouldShowTemp,      statusBar,       showTemp);
  
  CONNECT(palette,         shouldAttachColor,   colorPicker,     attach);
  CONNECT(palette,         shouldSetColor,      colorPicker,     setColor);
  CONNECT(palette,         shouldSetIndex,      colors,          setIndex);
  CONNECT(palette,         paletteColorChanged, editor,          composite);
  CONNECT(palette,         paletteColorChanged, colors,          changePaletteColors);
  CONNECT(palette,         paletteColorChanged, this,            modify);
}

void Window::saveFile() {
  const QString path = windowFilePath();
  if (path.isEmpty()) {
    saveFileDialog();
  } else {
    sprite.saveFile(path);
    setWindowModified(false);
    statusBar.showTemp("Saved!");
  }
}

void Window::saveFileDialog() {
  /*auto *dialog = new QFileDialog{this, "Save File"};
  CONNECT(dialog, fileSelected, this, setFileName);
  CONNECT(dialog, fileSelected, &timeline, saveFile);
  dialog->setNameFilter("Animera File (*.px2)");
  dialog->setDefaultSuffix("px2");
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->show();*/
  const QString saveFileName = QFileDialog::getSaveFileName(
    this,
    "Save File",
    QDir::homePath(),
    "Animera File (*.px2)"
  );
  if (!saveFileName.isEmpty()) {
    sprite.saveFile(saveFileName);
    setWindowFilePath(saveFileName);
    setWindowModified(false);
  }
}

void Window::exportDialog() {
  auto *dialog = new ExportDialog{this, sprite.getFormat()};
  CONNECT(dialog, exportSprite, sprite, exportSprite);
  CONNECT_LAMBDA(dialog, exportSprite, [this]{
    statusBar.showTemp("Exported!");
  });
  CONNECT_LAMBDA(dialog, exportSprite, [dialog]{
    delete dialog;
  });
  dialog->open();
}

void Window::closeEvent(QCloseEvent *) {
  static_cast<Application *>(QApplication::instance())->windowClosed(this);
}

#include "window.moc"
