//
//  tool colors widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 30/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_tool_colors_widget_hpp
#define animera_tool_colors_widget_hpp

#include <array>
#include "tool.hpp"
#include <QtWidgets/qwidget.h>

class ColorHandle;
class ActiveColorWidget;

class ToolColorsWidget final : public QWidget {
  Q_OBJECT
  
public:
  explicit ToolColorsWidget(QWidget *);

public Q_SLOTS:
  void initCanvas(Format);
  void setPalette(PaletteCSpan);
  void setIndex(int);
  void updatePaletteColors();

Q_SIGNALS:
  void colorsChanged(ToolColors);
  void shouldAttachColor(ColorHandle *);
  void shouldAttachIndex(int);
  void shouldShowNorm(std::string_view);

private Q_SLOTS:
  void changeColors();

private:
  ToolColors colors;
  std::array<ActiveColorWidget *, 3> widgets;
  
  static ToolColors getInitialColors(Format);
  void setupLayout();
  void connectSignals();
};

#endif
