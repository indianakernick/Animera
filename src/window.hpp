//
//  window.hpp
//  Animera
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef window_hpp
#define window_hpp

#include "sprite.hpp"
#include "undo object.hpp"
#include "clear object.hpp"
#include "export dialog.hpp"
#include "sample object.hpp"
#include "editor widget.hpp"
#include "palette widget.hpp"
#include "timeline widget.hpp"
#include <QtWidgets/qmenubar.h>
#include "status bar widget.hpp"
#include <QtWidgets/qsplitter.h>
#include "init canvas dialog.hpp"
#include "tool colors widget.hpp"
#include "tool select widget.hpp"
#include "color picker widget.hpp"
#include <QtWidgets/qmainwindow.h>

class Window final : public QMainWindow {
  Q_OBJECT

public:
  Window(QWidget *, QRect);
  
  void newFile(Format, QSize);
  void openFile(const QString &);

private Q_SLOTS:
  void modify();

private:
  QWidget central;
  QWidget bottom;
  QWidget right;
  QSplitter splitter;
  Sprite sprite;
  UndoObject undo;
  ClearObject clear;
  SampleObject sample;
  EditorWidget editor;
  PaletteWidget palette;
  ToolColorsWidget colors;
  ToolSelectWidget tools;
  TimelineWidget timeline;
  StatusBarWidget statusBar;
  ColorPickerWidget colorPicker;
  QMenuBar menubar;
  ExportDialog *exportDialog = nullptr;
  
  void setupLayouts();
  void initStyles();
  void populateMenubar();
  void connectSignals();
  
  void saveToPath(const QString &);
  void saveFile();
  void openSaveFileDialog();
  void exportSprite(const ExportOptions &);
  void openExportDialog();
  
  void closeEvent(QCloseEvent *) override;
};

#endif
