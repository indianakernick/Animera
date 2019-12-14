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
#include "settings.hpp"
#include <QtGui/qevent.h>
#include "quit dialog.hpp"
#include "application.hpp"
#include "global font.hpp"
#include "undo object.hpp"
#include "error dialog.hpp"
#include "status object.hpp"
#include "export dialog.hpp"
#include "sample object.hpp"
#include "editor widget.hpp"
#include <QtWidgets/qstyle.h>
#include "palette widget.hpp"
#include "timeline widget.hpp"
#include "separator widget.hpp"
#include <QtWidgets/qmenubar.h>
#include "status bar widget.hpp"
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qboxlayout.h>
#include "init canvas dialog.hpp"
#include "tool colors widget.hpp"
#include "tool select widget.hpp"
#include "color picker widget.hpp"
#include <QtWidgets/qfiledialog.h>
#include "tool param bar widget.hpp"
#include <QtWidgets/qdesktopwidget.h>

Window::Window(QWidget *parent, const Window *previous)
  : QMainWindow{parent} {
  setAttribute(Qt::WA_DeleteOnClose);
  createWidgets();
  resize(glob_window_size);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setFocusPolicy(Qt::StrongFocus);
  setPosition(previous);
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

void Window::openImage(const QString &path) {
  if (Error err = sprite.openImage(path); err) {
    QDesktopWidget *desktop = static_cast<Application *>(QApplication::instance())->desktop();
    (new ErrorDialog{desktop, "Image open error", err.msg()})->open();
  } else {
    setWindowModified(true);
    show();
  }
}

void Window::modify() {
  setWindowModified(true);
}

void Window::setPosition(const Window *previous) {
  const QRect desktop = QApplication::desktop()->availableGeometry(this);
  if (previous) {
  
#ifdef Q_OS_MACOS
    const QPoint offset = {21, 23};
#else
    const QPoint offset = toPoint(previous->geometry().top() - previous->pos().y());
#endif
  
    const QPoint newPos = previous->pos() + offset;
    const QPoint newCorner = toPoint(frameGeometry().size()) + newPos;
    if (desktop.contains(newCorner)) {
      move(newPos);
    } else {
      move(previous->pos());
    }
  } else {
    move(QStyle::alignedRect(
      Qt::LeftToRight,
      Qt::AlignCenter,
      frameGeometry().size(),
      desktop
    ).topLeft());
  }
}

void Window::createWidgets() {
  central = new QWidget{this};
  bottom = new QWidget{this};
  right = new QWidget{this};
  splitter = new QSplitter{this};
  undo = new UndoObject{this};
  sample = new SampleObject{this};
  editor = new EditorWidget{this};
  palette = new PaletteWidget{right};
  colors = new ToolColorsWidget{right};
  toolParams = new ToolParamBarWidget{bottom};
  tools = new ToolSelectWidget{this, toolParams};
  timeline = new TimelineWidget{bottom};
  statusBar = new StatusBarWidget{bottom};
  colorPicker = new ColorPickerWidget{right};
  status = new StatusObject{this};
  menubar = new QMenuBar{this};
}

void Window::setupLayouts() {
  auto *bottomLayout = new QGridLayout{bottom};
  bottomLayout->setContentsMargins(0, 0, 0, 0);
  bottomLayout->setSpacing(0);
  bottomLayout->addWidget(timeline, 0, 0, 1, 2);
  bottomLayout->addWidget(new HoriSeparator{bottom}, 1, 0, 1, 2);
  bottomLayout->addWidget(statusBar, 2, 0, 1, 1);
  bottomLayout->addWidget(toolParams, 2, 1, 1, 1);
  bottomLayout->setAlignment(Qt::AlignBottom);
  
  auto *rightLayout = new QVBoxLayout{right};
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(glob_margin);
  rightLayout->addWidget(colorPicker);
  rightLayout->addWidget(new HoriSeparator{right});
  rightLayout->addWidget(colors);
  rightLayout->addWidget(new HoriSeparator{right});
  rightLayout->addWidget(palette);
  rightLayout->addSpacing(glob_margin);
  right->setStyleSheet("background-color: " + glob_main.name());
  right->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  
  /*
  TODO: Qt bug
  Unfortunately, we can't use QDockWidget.
  When the title bar is removed with setTitleBar(new QWidget{this}),
  the contents of the dock is shifted up by one pixel.
  If the title bar is moved to the left with
  setFeatures(QDockWidget::DockWidgetVerticalTitleBar),
  the contents of the dock is shifted left by one pixel.
  I've tried numerous times to correct this issue and so far, the best option
  seems to be bypassing docks altogether.
  */
  
  auto *centralLayout = new QGridLayout{central};
  centralLayout->setContentsMargins(0, 0, 0, 0);
  centralLayout->setSpacing(0);
  centralLayout->addWidget(menubar, 0, 0, 1, 5);
  centralLayout->addWidget(new HoriSeparator{central}, 1, 0, 1, 5);
  centralLayout->addWidget(tools, 2, 0);
  centralLayout->addWidget(new VertSeparator{central}, 2, 1);
  centralLayout->addWidget(editor, 2, 2);
  centralLayout->addWidget(new VertSeparator{central}, 2, 3);
  centralLayout->addWidget(right, 2, 4);
  
  splitter->setStyleSheet(
    "QSplitter::handle {"
      "background-color: " + glob_border_color.name() + ";"
    "}"
  );
  splitter->setOrientation(Qt::Vertical);
  splitter->setHandleWidth(glob_border_width);
  splitter->setOpaqueResize(true);
  splitter->addWidget(central);
  splitter->addWidget(bottom);
  splitter->setCollapsible(0, false);
  splitter->setCollapsible(1, false);
  setCentralWidget(splitter);
}

void Window::initStyles() {
  setStyleSheet(
    "QMenuBar {"
      "background-color: " + glob_main.name() + ";"
      "color: " + glob_text_color.name() + ";"
      "outline-style: none;"
      "padding-left: " + QString::number(glob_margin + glob_border_width + glob_text_margin) + "px;"
    "}"
    
    "QMenuBar::item {"
      "padding: " + QString::number(glob_margin + glob_border_width + glob_text_margin) + "px;"
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
      "background-color: " + glob_border_color.name() + ";"
      "height: " + QString::number(glob_border_width) + "px;"
      "margin: 0;"
    "}"
    
    "QMenu::item {"
      "background-color: " + glob_main.name() + ";"
      "color: " + glob_text_color.name() + ";"
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
  menubar->setFont(getGlobalFont());
  menubar->setNativeMenuBar(false);
  
  auto *app = static_cast<Application *>(QApplication::instance());
  QMenu *file = menubar->addMenu("File");
  file->setFont(getGlobalFont());
  ADD_ACTION(file, "New", key_new_file, *app, newFileDialog);
  ADD_ACTION(file, "Open", key_open_file, *app, openFileDialog);
  ADD_ACTION(file, "Save", key_save_file, *this, saveFile);
  ADD_ACTION(file, "Save As", key_save_file_as, *this, saveFileDialog);
  ADD_ACTION(file, "Export", key_export_file, *this, exportDialog);
  ADD_ACTION(file, "Export Frame", key_export_frame, *this, exportFrameDialog);
  
  QMenu *layer = menubar->addMenu("Layer");
  layer->setFont(getGlobalFont());
  ADD_ACTION(layer, "New Layer", key_new_layer, sprite.timeline, insertLayer);
  ADD_ACTION(layer, "Delete Layer", key_delete_layer, sprite.timeline, removeLayer);
  ADD_ACTION(layer, "Move Layer Up", key_move_layer_up, sprite.timeline, moveLayerUp);
  ADD_ACTION(layer, "Move Layer Down", key_move_layer_down, sprite.timeline, moveLayerDown);
  //ADD_ACTION(layer, "Toggle Visibility", Qt::SHIFT + Qt::Key_V, sprite.timeline, toggleLayerVisible);
  // TODO: Maybe consider this
  // We have to keep the action in sync with the timeline
  /*{
    QAction *action = layer->addAction("Visible");
    action->setShortcut(Qt::SHIFT + Qt::Key_V);
    action->setCheckable(true);
    action->setChecked(true);
    CONNECT(action, triggered, &timeline, toggleLayerVisible);
  }*/
  layer->addSeparator();
  ADD_ACTION(layer, "Layer Above", key_layer_above, sprite.timeline, layerAbove);
  ADD_ACTION(layer, "Layer Below", key_layer_below, sprite.timeline, layerBelow);
  
  QMenu *frame = menubar->addMenu("Frame");
  frame->setFont(getGlobalFont());
  ADD_ACTION(frame, "New Frame", key_new_frame, sprite.timeline, insertFrame);
  ADD_ACTION(frame, "Delete Frame", key_delete_frame, sprite.timeline, removeFrame);
  frame->addSeparator();
  ADD_ACTION(frame, "Clear Cell", key_clear_cell, sprite.timeline, clearCell);
  ADD_ACTION(frame, "Extend Linked Cell", key_extend_cell, sprite.timeline, extendCell);
  ADD_ACTION(frame, "Split Linked Cell", key_split_cell, sprite.timeline, splitCell);
  frame->addSeparator();
  ADD_ACTION(frame, "Next Frame", key_next_frame, sprite.timeline, nextFrame);
  ADD_ACTION(frame, "Previous Frame", key_prev_frame, sprite.timeline, prevFrame);
  ADD_ACTION(frame, "Play Animation", key_play_anim, *timeline, toggleAnimation);
  
  QMenu *selection = menubar->addMenu("Selection");
  selection->setFont(getGlobalFont());
  ADD_ACTION(selection, "Clear", key_clear_selection, sprite.timeline, clearSelected);
  ADD_ACTION(selection, "Copy", key_copy_selection, sprite.timeline, copySelected);
  ADD_ACTION(selection, "Paste", key_paste_selection, sprite.timeline, pasteSelected);
  
  QMenu *pal = menubar->addMenu("Palette");
  pal->setFont(getGlobalFont());
  ADD_ACTION(pal, "Reset", key_reset_palette, *this, resetPalette);
  ADD_ACTION(pal, "Open", key_open_palette, *this, openPaletteDialog);
  ADD_ACTION(pal, "Save", key_save_palette, *this, savePaletteDialog);
  
  menubar->adjustSize();
}

#undef ADD_ACTION

void Window::connectSignals() {
  CONNECT(sprite.timeline, currCellChanged,     tools,           setCell);
  CONNECT(sprite.timeline, currCellChanged,     sample,          setCell);
  CONNECT(sprite.timeline, currCellChanged,     undo,            setCell);
  CONNECT(sprite.timeline, frameChanged,        editor,          setFrame);
  CONNECT(sprite.timeline, cellModified,        editor,          composite);
  CONNECT(sprite.timeline, currPosChanged,      timeline,        setCurrPos);
  CONNECT(sprite.timeline, selectionChanged,    timeline,        setSelection);
  CONNECT(sprite.timeline, visibilityChanged,   timeline,        setVisibility);
  CONNECT(sprite.timeline, nameChanged,         timeline,        setName);
  CONNECT(sprite.timeline, layerChanged,        timeline,        setLayer);
  CONNECT(sprite.timeline, frameCountChanged,   timeline,        setFrameCount);
  CONNECT(sprite.timeline, layerCountChanged,   timeline,        setLayerCount);
  CONNECT(sprite.timeline, delayChanged,        timeline,        setDelay);
  CONNECT(sprite.timeline, currPosChanged,      status,          setCurrPos);
  CONNECT(sprite.timeline, selectionChanged,    status,          setSelection);
  CONNECT(sprite.timeline, frameCountChanged,   status,          setFrameCount);
  CONNECT(sprite.timeline, layerCountChanged,   status,          setLayerCount);
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
  CONNECT(timeline,        currPosChanged,      sprite.timeline, setPos);
  CONNECT(timeline,        delayChanged,        sprite.timeline, setDelay);
  
  CONNECT(sprite,          canvasInitialized,   colorPicker,     initCanvas);
  CONNECT(sprite,          canvasInitialized,   colors,          initCanvas);
  CONNECT(sprite,          canvasInitialized,   editor,          initCanvas);
  CONNECT(sprite,          canvasInitialized,   palette,         initCanvas);
  CONNECT(sprite,          canvasInitialized,   tools,           initCanvas);
  CONNECT(sprite,          canvasInitialized,   sample,          initCanvas);
  
  CONNECT(sprite.palette,  paletteChanged,      palette,         setPalette);
  CONNECT(sprite.palette,  paletteChanged,      editor,          setPalette);
  CONNECT(sprite.palette,  paletteChanged,      tools,           setPalette);
  CONNECT(sprite.palette,  paletteChanged,      colors,          setPalette);
  
  CONNECT(tools,           cellModified,        editor,          composite);
  CONNECT(tools,           overlayModified,     editor,          compositeOverlay);
  CONNECT(tools,           shouldShowNorm,      statusBar,       showNorm);
  CONNECT(tools,           growRequested,       sprite.timeline, growCell);
  CONNECT(tools,           shrinkRequested,     sprite.timeline, shrinkCell);
  CONNECT(tools,           changingAction,      undo,            cellModified);
  CONNECT(tools,           changingAction,      this,            modify);
  CONNECT(tools,           lockRequested,       sprite.timeline, lock);
  CONNECT(tools,           unlockRequested,     sprite.timeline, unlock);
  
  CONNECT(editor,          overlayChanged,      tools,           setOverlay);
  CONNECT(editor,          mouseEnter,          tools,           mouseEnter);
  CONNECT(editor,          mouseLeave,          tools,           mouseLeave);
  CONNECT(editor,          mouseDown,           tools,           mouseDown);
  CONNECT(editor,          mouseMove,           tools,           mouseMove);
  CONNECT(editor,          mouseUp,             tools,           mouseUp);
  CONNECT(editor,          keyPress,            tools,           keyPress);
  CONNECT(editor,          mouseMove,           sample,          mouseMove);
  CONNECT(editor,          keyPress,            sample,          keyPress);
  CONNECT(editor,          keyPress,            undo,            keyPress);
  CONNECT(editor,          scaleChanged,        status,          setScale);
  
  CONNECT(colors,          colorsChanged,       tools,           setColors);
  CONNECT(colors,          shouldAttachColor,   colorPicker,     attach);
  CONNECT(colors,          shouldAttachIndex,   palette,         attachIndex);
  CONNECT(colors,          shouldShowNorm,      statusBar,       showNorm);
  
  CONNECT(sample,          shouldSetColor,      colorPicker,     setColor);
  CONNECT(sample,          shouldSetIndex,      colors,          setIndex);
  CONNECT(sample,          shouldSetIndex,      palette,         attachIndex);
  
  CONNECT(undo,            cellReverted,        editor,          composite);
  CONNECT(undo,            cellReverted,        this,            modify);
  CONNECT(undo,            shouldShowTemp,      statusBar,       showTemp);
  CONNECT(undo,            shouldClearCell,     sprite.timeline, clearCell);
  CONNECT(undo,            shouldGrowCell,      sprite.timeline, growCell);
  
  CONNECT(palette,         shouldAttachColor,   colorPicker,     attach);
  CONNECT(palette,         shouldSetColor,      colorPicker,     setColor);
  CONNECT(palette,         shouldSetIndex,      colors,          setIndex);
  CONNECT(palette,         paletteColorChanged, editor,          compositePalette);
  CONNECT(palette,         paletteColorChanged, colors,          changePaletteColors);
  CONNECT(palette,         paletteColorChanged, this,            modify);
  CONNECT(palette,         shouldShowNorm,      statusBar,       showNorm);
  
  CONNECT(colorPicker,     shouldShowNorm,      statusBar,       showNorm);
  
  CONNECT(status,          shouldShowPerm,      statusBar,       showPerm);
  CONNECT(status,          shouldShowPerm,      statusBar,       showTemp);
}

void Window::saveToPath(const QString &path) {
  sprite.optimize();
  if (Error err = sprite.saveFile(path); err) {
    auto *dialog = new ErrorDialog{this, "File save error", err.msg()};
    if (closeAfterSave) {
      CONNECT(dialog, finished, this, close);
    }
    dialog->open();
  } else {
    setWindowFilePath(path);
    setWindowModified(false);
    statusBar->showTemp("Saved!");
    if (closeAfterSave) {
      close();
    }
  }
}

void Window::saveFile() {
  const QString path = windowFilePath();
  if (path.isEmpty()) {
    saveFileDialog();
  } else {
    saveToPath(path);
  }
}

void Window::saveFileDialog() {
  auto *dialog = new QFileDialog{this};
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setNameFilter("Animera Sprite (*.animera)");
  dialog->setDefaultSuffix("animera");
  CONNECT(dialog, fileSelected, this, saveToPath);
  if (closeAfterSave) {
    CONNECT(dialog, rejected, this, close);
  }
  updateDirSettings(dialog, "Sprite Directory");
  dialog->open();
}

void Window::exportSprite(const ExportOptions &options) {
  if (Error err = sprite.exportSprite(options); err) {
    (new ErrorDialog{this, "Export error", err.msg()})->open();
  } else {
    statusBar->showTemp("Exported!");
  }
}

void Window::exportDialog() {
  if (!exporter) {
    exporter = new ExportDialog{this, sprite.getFormat()};
    CONNECT(exporter, exportSprite, this, exportSprite);
  }
  exporter->setLayers(sprite.timeline.getLayers());
  exporter->setFrames(sprite.timeline.getFrames());
  exporter->setPos(sprite.timeline.getPos());
  exporter->setSelection(sprite.timeline.getSelection());
  exporter->open();
}

void Window::exportFrame(const QString &path) {
  exportSprite(exportFrameOptions(path, getSpriteInfo(sprite)));
}

void Window::exportFrameDialog() {
  auto *dialog = new QFileDialog{this};
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setNameFilter("PNG Image (*.png)");
  dialog->setDefaultSuffix("png");
  CONNECT(dialog, fileSelected, this, exportFrame);
  updateDirSettings(dialog, "Export Directory");
  dialog->open();
}

void Window::openPalette(const QString &path) {
  if (Error err = sprite.palette.open(path); err) {
    (new ErrorDialog{this, "Palette open error", err.msg()})->open();
  } else {
    setWindowModified(true);
    palette->updatePalette();
  }
}

void Window::savePalette(const QString &path) {
  if (Error err = sprite.palette.save(path); err) {
    (new ErrorDialog{this, "Palette save error", err.msg()})->open();
  } else {
    statusBar->showTemp("Palette Saved!");
  }
}

void Window::openPaletteDialog() {
  auto *dialog = new QFileDialog{this};
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setNameFilter("PNG Image (*.png)");
  dialog->setFileMode(QFileDialog::ExistingFile);
  CONNECT(dialog, fileSelected, this, openPalette);
  updateDirSettings(dialog, "Palette Directory");
  dialog->open();
}

void Window::savePaletteDialog() {
  auto *dialog = new QFileDialog{this};
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setNameFilter("PNG Image (*.png)");
  dialog->setDefaultSuffix("png");
  CONNECT(dialog, fileSelected, this, savePalette);
  updateDirSettings(dialog, "Palette Directory");
  dialog->open();
}

void Window::resetPalette() {
  sprite.palette.reset();
  palette->updatePalette();
}

void Window::closeEvent(QCloseEvent *event) {
  auto *app = static_cast<Application *>(QApplication::instance());
  if (!app->isClosing() && showQuitDialog && isWindowModified()) {
    event->ignore();
    showQuitDialog = false;
    auto *quitter = new QuitDialog{this};
    CONNECT(quitter, shouldQuit, this, close);
    CONNECT_TYPE(quitter, shouldSave, this, saveFile, Qt::QueuedConnection);
    CONNECT_LAMBDA(quitter, shouldCancel, [this]() {
      showQuitDialog = true;
      closeAfterSave = false;
    });
    quitter->open();
  } else {
    app->windowClosed(this);
  }
}

#include "window.moc"
