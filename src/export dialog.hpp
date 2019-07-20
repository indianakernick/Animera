//
//  export dialog.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef export_dialog_hpp
#define export_dialog_hpp

#include <QtWidgets/qdialog.h>

class TextInputWidget;
class NumberInputWidget;
class TextPushButtonWidget;

enum class LayerSelect {
  current,
  all,
  all_composited
};

enum class FrameSelect {
  current,
  all
};

enum class ExportFormat {
  rgba,
  indexed,
  grayscale,
  monochrome
};

struct Line {
  int stride;
  int offset;
};

constexpr int apply(const Line line, const int x) {
  return x * line.stride + line.offset;
}

struct ExportOptions {
  QString name;
  QString directory;
  Line layerLine;
  Line frameLine;
  LayerSelect layerSelect;
  FrameSelect frameSelect;
};

class ExportDialog final : public QDialog {
  Q_OBJECT
  
public:
  explicit ExportDialog(QWidget *);
  
Q_SIGNALS:
  void exportSprite(ExportOptions);

private Q_SLOTS:
  void finalize();

private:
  ExportOptions options;
  TextInputWidget *name;
  NumberInputWidget *layerStride;
  NumberInputWidget *layerOffset;
  NumberInputWidget *frameStride;
  NumberInputWidget *frameOffset;
  TextPushButtonWidget *ok;
  TextPushButtonWidget *cancel;
  
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif
