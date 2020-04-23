//
//  export dialog.hpp
//  Animera
//
//  Created by Indiana Kernick on 20/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_export_dialog_hpp
#define animera_export_dialog_hpp

#include "image.hpp"
#include "dialog.hpp"
#include "export options.hpp"

class ComboBoxWidget;
class FileInputWidget;
class TextInputWidget;
class NumberInputWidget;
class TextPushButtonWidget;

class ExportDialog final : public Dialog {
  Q_OBJECT
  
public:
  ExportDialog(QWidget *, Format);
  
  void setLayers(LayerIdx);
  void setFrames(FrameIdx);
  void setPos(CelPos);
  void setSelection(CelRect);
  
Q_SIGNALS:
  void exportSprite(const ExportOptions &);

private Q_SLOTS:
  void submit();
  void updateFormatItems(const QString &);

private:
  Format format;
  LayerIdx layers;
  FrameIdx frames;
  CelPos pos;
  CelRect selection;
  
  TextInputWidget *name = nullptr;
  FileInputWidget *dir = nullptr;
  NumberInputWidget *layerStride = nullptr;
  NumberInputWidget *layerOffset = nullptr;
  NumberInputWidget *frameStride = nullptr;
  NumberInputWidget *frameOffset = nullptr;
  ComboBoxWidget *layerSelect = nullptr;
  ComboBoxWidget *frameSelect = nullptr;
  ComboBoxWidget *formatSelect = nullptr;
  ComboBoxWidget *composite = nullptr;
  ComboBoxWidget *visibility = nullptr;
  NumberInputWidget *scaleX = nullptr;
  NumberInputWidget *scaleY = nullptr;
  ComboBoxWidget *rotate = nullptr;
  TextPushButtonWidget *ok = nullptr;
  TextPushButtonWidget *cancel = nullptr;
  
  void addFormatOptions();
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif
