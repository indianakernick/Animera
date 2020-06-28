//
//  color picker widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 7/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_color_picker_widget_hpp
#define animera_color_picker_widget_hpp

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

Q_SIGNALS:
  void shouldShowNorm(std::string_view);

public Q_SLOTS:
  void initCanvas(Format);
  void attach(ColorHandle *);
  void setColor(PixelVar);

private Q_SLOTS:
  void setHandleColor(PixelVar);
  
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
