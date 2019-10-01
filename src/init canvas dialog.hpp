//
//  init canvas dialog.hpp
//  Animera
//
//  Created by Indi Kernick on 3/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef init_canvas_dialog_hpp
#define init_canvas_dialog_hpp

#include "image.hpp"
#include "dialog.hpp"

class FormatWidget;
class NumberInputWidget;
class TextPushButtonWidget;

class InitCanvasDialog final : public Dialog {
  Q_OBJECT
  
public:
  explicit InitCanvasDialog(QWidget *);
  
Q_SIGNALS:
  void canvasInitialized(Format, QSize);

private Q_SLOTS:
  void submit();

private:
  NumberInputWidget *width = nullptr;
  NumberInputWidget *height = nullptr;
  std::vector<FormatWidget *> formats;
  TextPushButtonWidget *ok = nullptr;
  TextPushButtonWidget *cancel = nullptr;
  
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif
