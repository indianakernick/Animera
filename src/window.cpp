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
#include "error dialog.hpp"
#include <QtWidgets/qstyle.h>
#include "separator widget.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qdesktopwidget.h>

Window::Window(QWidget *parent, const QRect desktop)
  : QMainWindow{parent},
    central{this},
    bottom{this},
    right{this},
    splitter{this},
    editor{this},
    palette{&right},
    colors{&right},
    tools{this},
    timeline{&bottom},
    statusBar{&bottom},
    colorPicker{&right},
    menubar{this} {
  resize(glob_window_size);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setFocusPolicy(Qt::StrongFocus);
  setGeometry(QStyle::alignedRect(
    Qt::LeftToRight,
    Qt::AlignCenter,
    size(),
    desktop
  ));
  initStyles();
  setupLayouts();
  populateMenubar();
  connectSignals();
}

void Window::newFile(const Format format, const QSize size) {
  sprite.newFile(format, size);
  setWindowModified(true);
  show();
}

void Window::openFile(const QString &path) {
  if (Error err = sprite.openFile(path); err) {
    QDesktopWidget *desktop = static_cast<Application *>(QApplication::instance())->desktop();
    (new ErrorDialog{desktop, "File open error", err.msg()})->open();
  } else {
    setWindowFilePath(path);
    setWindowModified(false);
    show();
  }
}

void Window::modify() {
  setWindowModified(true);
}

void Window::setupLayouts() {
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
  
  /*
  Unfortunately, we can't use QDockWidget.
  When the title bar is removed with setTitleBar(new QWidget{this}),
  the contents of the dock is shifted up by one pixel.
  If the title bar is moved to the left with
  setFeatures(QDockWidget::DockWidgetVerticalTitleBar),
  the contents of the dock is shifted left by one pixel.
  I've tried numerous times to correct this issue and so far, the best option
  seems to be bypassing docks altogether.
  */
  
  auto *centralLayout = new QGridLayout{&central};
  centralLayout->setContentsMargins(0, 0, 0, 0);
  centralLayout->setSpacing(0);
  centralLayout->addWidget(&menubar, 0, 0, 1, 5);
  centralLayout->addWidget(new HoriSeparator{&central}, 1, 0, 1, 5);
  centralLayout->addWidget(&tools, 2, 0);
  centralLayout->addWidget(new VertSeparator{&central}, 2, 1);
  centralLayout->addWidget(&editor, 2, 2);
  centralLayout->addWidget(new VertSeparator{&central}, 2, 3);
  centralLayout->addWidget(&right, 2, 4);
  
  splitter.setStyleSheet(
    "QSplitter::handle {"
      "background-color: " + glob_border_color.name() + ";"
    "}"
  );
  splitter.setOrientation(Qt::Vertical);
  splitter.setHandleWidth(glob_border_width);
  splitter.setOpaqueResize(true);
  splitter.addWidget(&central);
  splitter.addWidget(&bottom);
  splitter.setCollapsible(0, false);
  splitter.setCollapsible(1, false);
  setCentralWidget(&splitter);
}

void Window::initStyles() {
  setStyleSheet(
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
}

#define ADD_ACTION(MENU, NAME, SHORTCUT, WIDGET, MEMFN) do {                    \
  QAction *action = MENU->addAction(NAME);                                      \
  action->setShortcut(SHORTCUT);                                                \
  CONNECT(action, triggered, &WIDGET, MEMFN);                                   \
} while (0)

void Window::populateMenubar() {
  menubar.setFont(getGlobalFont());
  menubar.setNativeMenuBar(false);
  
  auto *app = static_cast<Application *>(QApplication::instance());
  QMenu *file = menubar.addMenu("File");
  file->setFont(getGlobalFont());
  ADD_ACTION(file, "New", QKeySequence::New, *app, newFileDialog);
  ADD_ACTION(file, "Open", QKeySequence::Open, *app, openFileDialog);
  ADD_ACTION(file, "Save", QKeySequence::Save, *this, saveFile);
  ADD_ACTION(file, "Save As", QKeySequence::SaveAs, *this, openSaveFileDialog);
  ADD_ACTION(file, "Export", Qt::CTRL + Qt::Key_E, *this, openExportDialog);
  
  QMenu *layer = menubar.addMenu("Layer");
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
  
  QMenu *frame = menubar.addMenu("Frame");
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
  
  QMenu *selection = menubar.addMenu("Selection");
  selection->setFont(getGlobalFont());
  ADD_ACTION(selection, "Clear", Qt::CTRL + Qt::Key_X, sprite.timeline, clearSelected);
  ADD_ACTION(selection, "Copy", Qt::CTRL + Qt::Key_C, sprite.timeline, copySelected);
  ADD_ACTION(selection, "Paste", Qt::CTRL + Qt::Key_V, sprite.timeline, pasteSelected);
  
  menubar.adjustSize();
}

#undef ADD_ACTION

void Window::connectSignals() {
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

void Window::saveToPath(const QString &path) {
  if (Error err = sprite.saveFile(path); err) {
    (new ErrorDialog{this, "File save error", err.msg()})->open();
  } else {
    setWindowFilePath(path);
    setWindowModified(false);
    statusBar.showTemp("Saved!");
  }
}

void Window::saveFile() {
  const QString path = windowFilePath();
  if (path.isEmpty()) {
    openSaveFileDialog();
  } else {
    saveToPath(path);
  }
}

void Window::openSaveFileDialog() {
  auto *dialog = new QFileDialog{this};
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setNameFilter("Animera File (*.animera)");
  dialog->setDefaultSuffix("animera");
  dialog->setDirectory(QDir::homePath() + "/my sprite.animera");
  CONNECT(dialog, fileSelected, this, saveToPath);
  dialog->open();
}

void Window::openExportDialog() {
  if (!exportDialog) {
    exportDialog = new ExportDialog{this, sprite.getFormat()};
    CONNECT(exportDialog, exportSprite, sprite, exportSprite);
    CONNECT_LAMBDA(exportDialog, exportSprite, [this]{
      statusBar.showTemp("Exported!");
    });
  }
  exportDialog->open();
}

void Window::closeEvent(QCloseEvent *) {
  static_cast<Application *>(QApplication::instance())->windowClosed(this);
}

#include "window.moc"
