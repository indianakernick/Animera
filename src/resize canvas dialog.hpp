//
//  resize canvas dialog.hpp
//  Animera
//
//  Created by Indiana Kernick on 27/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_resize_canvas_dialog_hpp
#define animera_resize_canvas_dialog_hpp

#include "dialog.hpp"

class NumberInputWidget;
class TextPushButtonWidget;

class ResizeCanvasDialog final : public Dialog {
  Q_OBJECT

public:
  explicit ResizeCanvasDialog(QWidget *);

  void setSize(QSize);

Q_SIGNALS:
  void canvasResized(QSize);

private Q_SLOTS:
  void submit();

private:
  NumberInputWidget *width = nullptr;
  NumberInputWidget *height = nullptr;
  TextPushButtonWidget *ok = nullptr;
  TextPushButtonWidget *cancel = nullptr;
  
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif
