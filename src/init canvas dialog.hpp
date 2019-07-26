//
//  init canvas dialog.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef init_canvas_dialog_hpp
#define init_canvas_dialog_hpp

#include "image.hpp"
#include <QtWidgets/qdialog.h>

class FormatWidget;
class NumberInputWidget;
class TextPushButtonWidget;

class InitCanvasDialog final : public QDialog {
  Q_OBJECT
  
public:
  explicit InitCanvasDialog(QWidget *);
  
Q_SIGNALS:
  void canvasInitialized(Format, QSize);

private Q_SLOTS:
  void submit();

private:
  NumberInputWidget *width;
  NumberInputWidget *height;
  std::vector<FormatWidget *> formats;
  TextPushButtonWidget *ok;
  TextPushButtonWidget *cancel;
  
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif
