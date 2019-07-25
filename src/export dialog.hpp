//
//  export dialog.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef export_dialog_hpp
#define export_dialog_hpp

#include "image.hpp"
#include "export options.hpp"
#include <QtWidgets/qdialog.h>

class ComboBoxWidget;
class FileInputWidget;
class TextInputWidget;
class NumberInputWidget;
class TextPushButtonWidget;

class ExportDialog final : public QDialog {
  Q_OBJECT
  
public:
  ExportDialog(QWidget *, Format);
  
Q_SIGNALS:
  void exportSprite(const ExportOptions &);

private Q_SLOTS:
  void finalize();
  void updateFormatItems(int);
  void setLayerSelect(int);
  void setFrameSelect(int);
  void setExportFormat(int);

private:
  Format format;
  ExportOptions options;
  TextInputWidget *name;
  FileInputWidget *dir;
  NumberInputWidget *layerStride;
  NumberInputWidget *layerOffset;
  NumberInputWidget *frameStride;
  NumberInputWidget *frameOffset;
  ComboBoxWidget *layerSelect;
  ComboBoxWidget *frameSelect;
  ComboBoxWidget *formatSelect;
  
  TextPushButtonWidget *ok;
  TextPushButtonWidget *cancel;
  
  void addFormatOptions();
  void createWidgets();
  void setupLayout();
  void connectSignals();
  void initDefault();
};

#endif
