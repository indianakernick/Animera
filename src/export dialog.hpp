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
#include "abstract export params.hpp"

class ComboBoxWidget;
class FileInputWidget;
class TextInputWidget;
class NumberInputWidget;
class TextPushButtonWidget;

class ExportDialog final : public Dialog {
  Q_OBJECT
  
public:
  ExportDialog(QWidget *, Format);
  
  void setPath(const QString &);
  void setInfo(const ExportSpriteInfo &);
  
Q_SIGNALS:
  void exportSprite(const ExportParams &);

private Q_SLOTS:
  void submit();
  void updateFormatItems(const QString &);

private:
  Format format;
  ExportSpriteInfo info; // format is also in here
  
  TextInputWidget *name = nullptr;
  FileInputWidget *dir = nullptr;
  ComboBoxWidget *layerName = nullptr;
  ComboBoxWidget *groupName = nullptr;
  ComboBoxWidget *frameName = nullptr;
  ComboBoxWidget *layerSelect = nullptr;
  ComboBoxWidget *frameSelect = nullptr;
  ComboBoxWidget *formatSelect = nullptr;
  ComboBoxWidget *composite = nullptr;
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
