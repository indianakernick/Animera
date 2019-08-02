//
//  color picker widget.hpp
//  Animera
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef color_picker_widget_hpp
#define color_picker_widget_hpp

#include <memory>
#include "image.hpp"
#include "color handle.hpp"
#include <QtWidgets/qwidget.h>
#include "color picker impl.hpp"

class LabelWidget;

class ColorPickerWidget final : public QWidget {
  Q_OBJECT

public:
  explicit ColorPickerWidget(QWidget *);

public Q_SLOTS:
  void initCanvas(Format);
  void attach(ColorHandle *);
  void setColor(QRgb);

private Q_SLOTS:
  void setHandleColor(QRgb);
  
private:
  void setImpl(Format);
  void init();
  void setupLayout();
  void connectSignals();

private:
  ColorHandle *handle = nullptr;
  std::unique_ptr<ColorPickerImpl> impl;
  LabelWidget *nameLabel = nullptr;
};

#endif
