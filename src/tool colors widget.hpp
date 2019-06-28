//
//  tool colors widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 30/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef tool_colors_widget_hpp
#define tool_colors_widget_hpp

#include "tool.hpp"
#include <QtWidgets/qwidget.h>

class ColorHandle;
class ActiveColorWidget;

class ToolColorsWidget final : public QWidget {
  Q_OBJECT
  
public:
  explicit ToolColorsWidget(QWidget *);

public Q_SLOTS:
  void initCanvas();

Q_SIGNALS:
  void colorsChanged(ToolColors);
  void attachColor(ColorHandle *);

private:
  ToolColors colors;
  ActiveColorWidget *primary;
  ActiveColorWidget *secondary;
  ActiveColorWidget *erase;
  
  void setupLayout();
  template <auto>
  auto toggleColor(bool);
  void connectSignals();

private Q_SLOTS:
  void changeColors();
};

#endif
