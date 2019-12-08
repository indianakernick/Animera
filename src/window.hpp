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
#include <QtWidgets/qmainwindow.h>

class QWidget;
class QSplitter;
class UndoObject;
class SampleObject;
class EditorWidget;
class PaletteWidget;
class ToolColorsWidget;
class ToolSelectWidget;
class ToolParamBarWidget;
class TimelineWidget;
class StatusBarWidget;
class ColorPickerWidget;
class QMenuBar;
class ExportDialog;

class Window final : public QMainWindow {
  Q_OBJECT

public:
  Window(QWidget *, QRect);
  
  void newFile(Format, QSize);
  void openFile(const QString &);
  void openImage(const QString &);

private Q_SLOTS:
  void modify();

private:
  Sprite sprite;
  QWidget *central = nullptr;
  QWidget *bottom = nullptr;
  QWidget *right = nullptr;
  QSplitter *splitter = nullptr;
  UndoObject *undo = nullptr;
  SampleObject *sample = nullptr;
  EditorWidget *editor = nullptr;
  PaletteWidget *palette = nullptr;
  ToolColorsWidget *colors = nullptr;
  ToolSelectWidget *tools = nullptr;
  ToolParamBarWidget *toolParams = nullptr;
  TimelineWidget *timeline = nullptr;
  StatusBarWidget *statusBar = nullptr;
  ColorPickerWidget *colorPicker = nullptr;
  QMenuBar *menubar = nullptr;
  ExportDialog *exporter = nullptr;
  
  void createWidgets();
  void setupLayouts();
  void initStyles();
  void populateMenubar();
  void connectSignals();
  
  void saveToPath(const QString &);
  void saveFile();
  void saveFileDialog();
  void exportSprite(const ExportOptions &);
  void exportDialog();
  void exportFrame(const QString &);
  void exportFrameDialog();
  void openPalette(const QString &);
  void savePalette(const QString &);
  void openPaletteDialog();
  void savePaletteDialog();
  void resetPalette();
  
  void closeEvent(QCloseEvent *) override;
};

#endif
