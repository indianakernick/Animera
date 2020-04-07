//
//  tool widgets.hpp
//  Animera
//
//  Created by Indiana Kernick on 24/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef tool_widgets_hpp
#define tool_widgets_hpp

#include "brush tool.hpp"
#include "select tools.hpp"
#include "gradient tool.hpp"
#include "transform tools.hpp"
#include "flood fill tool.hpp"
#include "drag paint tools.hpp"
#include "tool param widget.hpp"

class ComboBoxWidget;
class NumberInputWidget;

class BrushToolWidget final : public ToolParamWidget {
public:
  using impl = BrushTool;
  static constexpr const char icon_name[] = "brush";
  static constexpr const char tooltip[] = "Brush Tool";

  using ToolParamWidget::ToolParamWidget;

  void createWidgets();
  void setupLayout();
  void connectSignals(BrushTool *);

private:
  NumberInputWidget *radius = nullptr;
  ComboBoxWidget *symmetry = nullptr;
};

class FloodFillToolWidget final : public ToolParamWidget {
public:
  using impl = FloodFillTool;
  static constexpr const char icon_name[] = "flood fill";
  static constexpr const char tooltip[] = "Flood Fill Tool";

  using ToolParamWidget::ToolParamWidget;
};

class RectangleSelectToolWidget final : public ToolParamWidget {
public:
  using impl = RectangleSelectTool;
  static constexpr const char icon_name[] = "rectangle select";
  static constexpr const char tooltip[] = "Rectangle Select Tool";

  using ToolParamWidget::ToolParamWidget;
};

class PolygonSelectToolWidget final : public ToolParamWidget {
public:
  using impl = PolygonSelectTool;
  static constexpr const char icon_name[] = "polygon select";
  static constexpr const char tooltip[] = "Polygon Select Tool";

  using ToolParamWidget::ToolParamWidget;
};

class WandSelectToolWidget final : public ToolParamWidget {
public:
  using impl = WandSelectTool;
  static constexpr const char icon_name[] = "wand select";
  static constexpr const char tooltip[] = "Wand Select Tool";

  using ToolParamWidget::ToolParamWidget;
};

class LineToolWidget final : public ToolParamWidget {
public:
  using impl = LineTool;
  static constexpr const char icon_name[] = "line";
  static constexpr const char tooltip[] = "Line Tool";

  using ToolParamWidget::ToolParamWidget;
  
  void createWidgets();
  void setupLayout();
  void connectSignals(LineTool *);

private:
  NumberInputWidget *radius = nullptr;
};

class StrokedCircleToolWidget final : public ToolParamWidget {
public:
  using impl = StrokedCircleTool;
  static constexpr const char icon_name[] = "stroked circle";
  static constexpr const char tooltip[] = "Stroked Circle Tool";

  using ToolParamWidget::ToolParamWidget;
  
  void createWidgets();
  void setupLayout();
  void connectSignals(StrokedCircleTool *);

private:
  NumberInputWidget *thickness = nullptr;
  ComboBoxWidget *shape = nullptr;
};

class FilledCircleToolWidget final : public ToolParamWidget {
public:
  using impl = FilledCircleTool;
  static constexpr const char icon_name[] = "filled circle";
  static constexpr const char tooltip[] = "Filled Circle Tool";

  using ToolParamWidget::ToolParamWidget;
  
  void createWidgets();
  void setupLayout();
  void connectSignals(FilledCircleTool *);

private:
  ComboBoxWidget *shape = nullptr;
};

class StrokedRectangleToolWidget final : public ToolParamWidget {
public:
  using impl = StrokedRectangleTool;
  static constexpr const char icon_name[] = "stroked rectangle";
  static constexpr const char tooltip[] = "Stroked Rectangle Tool";

  using ToolParamWidget::ToolParamWidget;
  
  void createWidgets();
  void setupLayout();
  void connectSignals(StrokedRectangleTool *);

private:
  NumberInputWidget *thickness = nullptr;
};

class FilledRectangleToolWidget final : public ToolParamWidget {
public:
  using impl = FilledRectangleTool;
  static constexpr const char icon_name[] = "filled rectangle";
  static constexpr const char tooltip[] = "Filled Rectangle Tool";

  using ToolParamWidget::ToolParamWidget;
};

class LinearGradientToolWidget final : public ToolParamWidget {
public:
  using impl = LinearGradientTool;
  static constexpr const char icon_name[] = "linear gradient";
  static constexpr const char tooltip[] = "Linear Gradient Tool";

  using ToolParamWidget::ToolParamWidget;
};

class TranslateToolWidget final : public ToolParamWidget {
public:
  using impl = TranslateTool;
  static constexpr const char icon_name[] = "translate";
  static constexpr const char tooltip[] = "Translate Tool";

  using ToolParamWidget::ToolParamWidget;
};

class FlipToolWidget final : public ToolParamWidget {
public:
  using impl = FlipTool;
  static constexpr const char icon_name[] = "flip";
  static constexpr const char tooltip[] = "Flip Tool";

  using ToolParamWidget::ToolParamWidget;
};

class RotateToolWidget final : public ToolParamWidget {
public:
  using impl = RotateTool;
  static constexpr const char icon_name[] = "rotate";
  static constexpr const char tooltip[] = "Rotate Tool";

  using ToolParamWidget::ToolParamWidget;
};

#endif
