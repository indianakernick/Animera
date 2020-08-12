//
//  window.cpp
//  Animera
//
//  Created by Indiana Kernick on 10/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "window.hpp"

#include "connect.hpp"
#include "settings.hpp"
#include <QtGui/qevent.h>
#include "quit dialog.hpp"
#include "keys dialog.hpp"
#include "application.hpp"
#include "global font.hpp"
#include "undo object.hpp"
#include "config keys.hpp"
#include "error dialog.hpp"
#include "config colors.hpp"
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
#include "resize canvas dialog.hpp"
#include "export texture atlas.hpp"
#include "tool param bar widget.hpp"
#include <QtWidgets/qdesktopwidget.h>
#include "abstract export params.hpp"

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
  anim.newFile(format, size);
  show();
}

void Window::openFile(const QString &path) {
  if (Error err = anim.openFile(path); err) {
    QDesktopWidget *desktop = static_cast<Application *>(QApplication::instance())->desktop();
    (new ErrorDialog{desktop, "File open error", err.msg()})->open();
  } else {
    setWindowFilePath(path);
    setWindowModified(false);
    show();
  }
}

void Window::openImage(const QString &path) {
  if (Error err = anim.openImage(path); err) {
    QDesktopWidget *desktop = static_cast<Application *>(QApplication::instance())->desktop();
    (new ErrorDialog{desktop, "Image open error", err.msg()})->open();
  } else {
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
  right->setStyleSheet("background-color:" + glob_main.name());
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
      "border: none"
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
      "padding: " + QString::number(glob_text_margin + glob_margin) + "px;"
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

namespace {

class ToggleLayerVis final : public QObject {
  Q_OBJECT

public:
  explicit ToggleLayerVis(QAction *action)
    : QObject{action}, action{action} {}
  
  void toggleVis(Timeline &timeline) {
    timeline.setVisibility(layer, !visible[+layer]);
  }
  
public Q_SLOTS:
  void setPos(const CelPos pos) {
    if (layer == pos.l) return;
    layer = pos.l;
    updateText();
  }
  
  void setVisibility(const LayerIdx idx, const bool visibility) {
    visible[+idx] = visibility;
    updateText();
  }
  
  void setLayerCount(const LayerIdx count) {
    visible.resize(+count, true);
  }
  
private:
  QAction *action;
  LayerIdx layer = LayerIdx{};
  std::vector<bool> visible;
  
  void updateText() {
    if (visible[+layer]) {
      action->setText("Hide Layer");
    } else {
      action->setText("Show Layer");
    }
  }
};

}

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
  ADD_ACTION(file, "Resize", {}, *this, resizeDialog);
  file->addSeparator();
  ADD_ACTION(file, "Export", key_export_file, *this, exportDialog);
  ADD_ACTION(file, "Export Frame", key_export_frame, *this, exportFrameDialog);
  ADD_ACTION(file, "Export Cel", key_export_cel, *this, exportCelDialog);
  file->addSeparator();
  // ADD_ACTION(file, "Import", QString{"CTRL+I"}, *this, exportDialog);
  ADD_ACTION(file, "Import Cel", key_import_cel, *this, importCelDialog);
  
  QMenu *layer = menubar->addMenu("Layer");
  layer->setFont(getGlobalFont());
  ADD_ACTION(layer, "New Layer", key_new_layer, anim.timeline, insertLayer);
  ADD_ACTION(layer, "Delete Layer", key_delete_layer, anim.timeline, removeLayer);
  ADD_ACTION(layer, "Move Layer Up", key_move_layer_up, anim.timeline, moveLayerUp);
  ADD_ACTION(layer, "Move Layer Down", key_move_layer_down, anim.timeline, moveLayerDown);
  
  {
    QAction *action = layer->addAction("Hide Layer");
    action->setShortcut(key_toggle_layer_vis);
    auto *toggle = new ToggleLayerVis{action};
    CONNECT_LAMBDA(action, triggered, [toggle, this] {
      toggle->toggleVis(anim.timeline);
    });
    CONNECT(anim.timeline, posChanged,    toggle, setPos);
    CONNECT(anim.timeline, visibilityChanged, toggle, setVisibility);
    CONNECT(anim.timeline, layerCountChanged, toggle, setLayerCount);
  }
  
  {
    QAction *action = layer->addAction("Isolate Layer");
    action->setShortcut(key_isolate_layer);
    CONNECT_LAMBDA(action, triggered, [this] {
      anim.timeline.isolateVisibility(anim.timeline.getPos().l);
    });
  }
  
  layer->addSeparator();
  ADD_ACTION(layer, "Layer Above", key_layer_above, anim.timeline, layerAbove);
  ADD_ACTION(layer, "Layer Below", key_layer_below, anim.timeline, layerBelow);
  
  QMenu *group = menubar->addMenu("Group");
  group->setFont(getGlobalFont());
  ADD_ACTION(group, "Split to Left", {}, anim.timeline, splitGroupLeft);
  ADD_ACTION(group, "Split to Right", {}, anim.timeline, splitGroupRight);
  ADD_ACTION(group, "Merge with Left", {}, anim.timeline, mergeGroupLeft);
  ADD_ACTION(group, "Merge with Right", {}, anim.timeline, mergeGroupRight);
  
  QMenu *frame = menubar->addMenu("Frame");
  frame->setFont(getGlobalFont());
  ADD_ACTION(frame, "New Frame", key_new_frame, anim.timeline, insertFrame);
  ADD_ACTION(frame, "Delete Frame", key_delete_frame, anim.timeline, removeFrame);
  frame->addSeparator();
  ADD_ACTION(frame, "Clear Cel", key_clear_cel, anim.timeline, clearCel);
  ADD_ACTION(frame, "Extend Linked Cel", key_extend_cel, anim.timeline, extendCel);
  ADD_ACTION(frame, "Split Linked Cel", key_split_cel, anim.timeline, splitCel);
  frame->addSeparator();
  ADD_ACTION(frame, "Next Frame", key_next_frame, anim.timeline, nextFrame);
  ADD_ACTION(frame, "Previous Frame", key_prev_frame, anim.timeline, prevFrame);
  
  {
    QAction *action = frame->addAction("Play Animation");
    action->setShortcut(key_play_anim);
    CONNECT(action, triggered, timeline, toggleAnimation);
    CONNECT_LAMBDA(timeline, shouldToggleAnimation, [action](const bool playing) {
      if (playing) {
        action->setText("Pause Animation");
      } else {
        action->setText("Play Animation");
      }
    });
  }
  
  QMenu *selection = menubar->addMenu("Selection");
  selection->setFont(getGlobalFont());
  ADD_ACTION(selection, "Clear", key_clear_selection, anim.timeline, clearSelected);
  ADD_ACTION(selection, "Copy", key_copy_selection, anim.timeline, copySelected);
  ADD_ACTION(selection, "Paste", key_paste_selection, anim.timeline, pasteSelected);
  
  QMenu *pal = menubar->addMenu("Palette");
  pal->setFont(getGlobalFont());
  ADD_ACTION(pal, "Reset", key_reset_palette, *this, resetPalette);
  ADD_ACTION(pal, "Open", key_open_palette, *this, openPaletteDialog);
  ADD_ACTION(pal, "Save", key_save_palette, *this, savePaletteDialog);
  
  QMenu *help = menubar->addMenu("Help");
  help->setFont(getGlobalFont());
  ADD_ACTION(help, "Key Bindings", {}, *this, keysDialog);
  
  menubar->adjustSize();
}

#undef ADD_ACTION

void Window::connectSignals() {
  CONNECT(anim.timeline,   celImageChanged,              tools,         setCelImage);
  CONNECT(anim.timeline,   celImageChanged,              sample,        setCelImage);
  CONNECT(anim.timeline,   celImageChanged,              undo,          setCelImage);
  CONNECT(anim.timeline,   frameChanged,                 editor,        setFrame);
  CONNECT(anim.timeline,   celImageModified,             editor,        composite);
  CONNECT(anim.timeline,   posChanged,                   timeline,      setPos);
  CONNECT(anim.timeline,   selectionChanged,             timeline,      setSelection);
  CONNECT(anim.timeline,   groupChanged,                 timeline,      setGroup);
  CONNECT(anim.timeline,   groupNameChanged,             timeline,      setGroupName);
  CONNECT(anim.timeline,   groupArrayChanged,            timeline,      setGroupArray);
  CONNECT(anim.timeline,   visibilityChanged,            timeline,      setLayerVisibility);
  CONNECT(anim.timeline,   layerNameChanged,             timeline,      setLayerName);
  CONNECT(anim.timeline,   layerCelsChanged,             timeline,      setLayerCels);
  CONNECT(anim.timeline,   frameCountChanged,            timeline,      setFrameCount);
  CONNECT(anim.timeline,   layerCountChanged,            timeline,      setLayerCount);
  CONNECT(anim.timeline,   delayChanged,                 timeline,      setDelay);
  CONNECT(anim.timeline,   posChanged,                   status,        setPos);
  CONNECT(anim.timeline,   selectionChanged,             status,        setSelection);
  CONNECT(anim.timeline,   frameCountChanged,            status,        setFrameCount);
  CONNECT(anim.timeline,   layerCountChanged,            status,        setLayerCount);
  CONNECT(anim.timeline,   modified,                     this,          modify);
  
  CONNECT(timeline,        shouldSetLayerVisibility,     anim.timeline, setVisibility);
  CONNECT(timeline,        shouldIsolateLayerVisibility, anim.timeline, isolateVisibility);
  CONNECT(timeline,        shouldSetLayerName,           anim.timeline, setLayerName);
  CONNECT(timeline,        shouldNextFrame,              anim.timeline, nextFrame);
  CONNECT(timeline,        shouldInsertLayer,            anim.timeline, insertLayer);
  CONNECT(timeline,        shouldRemoveLayer,            anim.timeline, removeLayer);
  CONNECT(timeline,        shouldMoveLayerUp,            anim.timeline, moveLayerUp);
  CONNECT(timeline,        shouldMoveLayerDown,          anim.timeline, moveLayerDown);
  CONNECT(timeline,        shouldExtendCel,              anim.timeline, extendCel);
  CONNECT(timeline,        shouldSplitCel,               anim.timeline, splitCel);
  CONNECT(timeline,        shouldBeginSelection,         anim.timeline, beginSelection);
  CONNECT(timeline,        shouldContinueSelection,      anim.timeline, continueSelection);
  CONNECT(timeline,        shouldEndSelection,           anim.timeline, endSelection);
  CONNECT(timeline,        shouldClearSelection,         anim.timeline, clearSelection);
  CONNECT(timeline,        shouldSetGroup,               anim.timeline, setGroup);
  CONNECT(timeline,        shouldSetGroupName,           anim.timeline, setGroupName);
  CONNECT(timeline,        shouldMoveGroup,              anim.timeline, moveGroup);
  CONNECT(timeline,        shouldSetPos,                 anim.timeline, setPos);
  CONNECT(timeline,        shouldSetDelay,               anim.timeline, setDelay);
  
  CONNECT(anim,            canvasInitialized,            colorPicker,   initCanvas);
  CONNECT(anim,            canvasInitialized,            colors,        initCanvas);
  CONNECT(anim,            canvasInitialized,            editor,        initCanvas);
  CONNECT(anim,            canvasInitialized,            palette,       initCanvas);
  CONNECT(anim,            canvasInitialized,            tools,         initCanvas);
  CONNECT(anim,            canvasInitialized,            sample,        initCanvas);
  
  CONNECT(anim,            canvasResized,                editor,        resizeCanvas);
  CONNECT(anim,            canvasResized,                tools,         resizeCanvas);
  CONNECT(anim,            canvasResized,                this,          modify);
  
  CONNECT(anim.palette,    paletteChanged,               palette,       setPalette);
  CONNECT(anim.palette,    paletteChanged,               editor,        setPalette);
  CONNECT(anim.palette,    paletteChanged,               tools,         setPalette);
  CONNECT(anim.palette,    paletteChanged,               colors,        setPalette);
  
  CONNECT(tools,           celImageModified,             editor,        composite);
  CONNECT(tools,           overlayModified,              editor,        compositeOverlay);
  CONNECT(tools,           shouldShowNorm,               statusBar,     showNorm);
  CONNECT(tools,           shouldGrowCelImage,           anim.timeline, growCelImage);
  CONNECT(tools,           shouldShrinkCelImage,         anim.timeline, shrinkCelImage);
  CONNECT(tools,           changingAction,               undo,          celImageModified);
  CONNECT(tools,           changingAction,               this,          modify);
  CONNECT(tools,           shouldLock,                   anim.timeline, lock);
  CONNECT(tools,           shouldUnlock,                 anim.timeline, unlock);
  
  CONNECT(editor,          overlayChanged,               tools,         setOverlay);
  CONNECT(editor,          mouseEnter,                   tools,         mouseEnter);
  CONNECT(editor,          mouseLeave,                   tools,         mouseLeave);
  CONNECT(editor,          mouseDown,                    tools,         mouseDown);
  CONNECT(editor,          mouseMove,                    tools,         mouseMove);
  CONNECT(editor,          mouseUp,                      tools,         mouseUp);
  CONNECT(editor,          keyPress,                     tools,         keyPress);
  CONNECT(editor,          mouseMove,                    sample,        mouseMove);
  CONNECT(editor,          keyPress,                     sample,        keyPress);
  CONNECT(editor,          keyPress,                     undo,          keyPress);
  CONNECT(editor,          scaleChanged,                 status,        setScale);
  
  CONNECT(colors,          colorsChanged,                tools,         setColors);
  CONNECT(colors,          shouldAttachColor,            colorPicker,   attach);
  CONNECT(colors,          shouldAttachIndex,            palette,       attachIndex);
  CONNECT(colors,          shouldShowNorm,               statusBar,     showNorm);
  
  CONNECT(sample,          shouldSetColor,               colorPicker,   setColor);
  CONNECT(sample,          shouldSetIndex,               colors,        setIndex);
  CONNECT(sample,          shouldSetIndex,               palette,       attachIndex);
  
  CONNECT(undo,            celImageReverted,             editor,        composite);
  CONNECT(undo,            celImageReverted,             this,          modify);
  CONNECT(undo,            shouldShowTemp,               statusBar,     showTemp);
  CONNECT(undo,            shouldClearCel,               anim.timeline, clearCel);
  CONNECT(undo,            shouldGrowCelImage,           anim.timeline, growCelImage);
  
  CONNECT(palette,         shouldAttachColor,            colorPicker,   attach);
  CONNECT(palette,         shouldSetColor,               colorPicker,   setColor);
  CONNECT(palette,         shouldSetIndex,               colors,        setIndex);
  CONNECT(palette,         paletteColorChanged,          editor,        compositePalette);
  CONNECT(palette,         paletteColorChanged,          colors,        updatePaletteColors);
  CONNECT(palette,         paletteColorChanged,          this,          modify);
  CONNECT(palette,         shouldShowNorm,               statusBar,     showNorm);
  
  CONNECT(colorPicker,     shouldShowNorm,               statusBar,     showNorm);
  
  CONNECT(status,          shouldShowPerm,               statusBar,     showPerm);
  CONNECT(status,          shouldShowApnd,               statusBar,     showApnd);
}

void Window::saveToPath(const QString &path) {
  anim.optimize();
  if (Error err = anim.saveFile(path); err) {
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
  dialog->setNameFilter("Animera Animation (*.animera)");
  dialog->setDefaultSuffix("animera");
  CONNECT(dialog, fileSelected, this, saveToPath);
  if (closeAfterSave) {
    CONNECT(dialog, rejected, this, close);
  }
  updateDirSettings(dialog, pref_animation_dir);
  dialog->open();
}

void Window::exportAnimation(const ExportParams &params) {
  if (Error err = exportTextureAtlas(params, anim); err) {
    (new ErrorDialog{this, "Export error", err.msg()})->open();
  } else {
    statusBar->showTemp("Exported!");
  }
}

void Window::exportDialog() {
  if (!exporter) {
    exporter = new ExportDialog{this, anim.getFormat()};
    CONNECT(exporter, exportAnimation, this, exportAnimation);
  }
  exporter->setPath(windowFilePath());
  exporter->setInfo(getAnimationInfo(anim));
  exporter->open();
}

void Window::exportFrame(const QString &path) {
  exportAnimation(exportFrameParams(getAnimationInfo(anim), path));
}

void Window::exportFrameDialog() {
  auto *dialog = new QFileDialog{this};
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setNameFilter("PNG Image (*.png)");
  dialog->setDefaultSuffix("png");
  CONNECT(dialog, fileSelected, this, exportFrame);
  updateDirSettings(dialog, pref_export_dir);
  dialog->open();
}

void Window::exportCel(const QString &path) {
  exportAnimation(exportCelParams(getAnimationInfo(anim), path));
}

void Window::exportCelDialog() {
  // This is nearly identical to exportFrameDialog
  // TODO: Can we move these dialog functions out of the window class?
  auto *dialog = new QFileDialog{this};
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setNameFilter("PNG Image (*.png)");
  dialog->setDefaultSuffix("png");
  CONNECT(dialog, fileSelected, this, exportCel);
  updateDirSettings(dialog, pref_export_dir);
  dialog->open();
}

void Window::importCel(const QString &path) {
  if (Error err = anim.timeline.importImage(path); err) {
    (new ErrorDialog{this, "Import error", err.msg()})->open();
  }
}

void Window::importCelDialog() {
  // TODO: This is very similar to openPaletteDialog
  auto *dialog = new QFileDialog{this};
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setNameFilter("PNG Image (*.png)");
  dialog->setFileMode(QFileDialog::ExistingFile);
  CONNECT(dialog, fileSelected, this, importCel);
  updateDirSettings(dialog, pref_import_dir);
  dialog->open();
}

void Window::openPalette(const QString &path) {
  if (Error err = anim.palette.open(path); err) {
    (new ErrorDialog{this, "Palette open error", err.msg()})->open();
  } else {
    setWindowModified(true);
    palette->updatePalette();
  }
}

void Window::savePalette(const QString &path) {
  if (Error err = anim.palette.save(path); err) {
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
  updateDirSettings(dialog, pref_palette_dir);
  dialog->open();
}

void Window::savePaletteDialog() {
  auto *dialog = new QFileDialog{this};
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setNameFilter("PNG Image (*.png)");
  dialog->setDefaultSuffix("png");
  CONNECT(dialog, fileSelected, this, savePalette);
  updateDirSettings(dialog, pref_palette_dir);
  dialog->open();
}

void Window::resetPalette() {
  anim.palette.reset();
  palette->updatePalette();
}

void Window::keysDialog() {
  auto *app = static_cast<QApplication *>(QApplication::instance());
  auto *dialog = new KeysDialog{app->desktop()};
  dialog->open();
}

void Window::resizeDialog() {
  auto *dialog = new ResizeCanvasDialog{this};
  CONNECT(dialog, canvasResized, anim, resizeCanvas);
  dialog->setSize(anim.getSize());
  dialog->open();
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
