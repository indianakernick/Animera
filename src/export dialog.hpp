//
//  export dialog.hpp
//  Animera
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef export_dialog_hpp
#define export_dialog_hpp

#include "image.hpp"
#include "dialog.hpp"

struct ExportOptions;
class ComboBoxWidget;
class FileInputWidget;
class TextInputWidget;
class NumberInputWidget;
class TextPushButtonWidget;

class ExportDialog final : public Dialog {
  Q_OBJECT
  
public:
  ExportDialog(QWidget *, Format);
  
Q_SIGNALS:
  void exportSprite(const ExportOptions &);

private Q_SLOTS:
  void submit();
  void updateFormatItems(int);

private:
  Format format;
  TextInputWidget *name = nullptr;
  FileInputWidget *dir = nullptr;
  NumberInputWidget *layerStride = nullptr;
  NumberInputWidget *layerOffset = nullptr;
  NumberInputWidget *frameStride = nullptr;
  NumberInputWidget *frameOffset = nullptr;
  ComboBoxWidget *layerSelect = nullptr;
  ComboBoxWidget *frameSelect = nullptr;
  ComboBoxWidget *formatSelect = nullptr;
  TextPushButtonWidget *ok = nullptr;
  TextPushButtonWidget *cancel = nullptr;
  
  void addFormatOptions();
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif
