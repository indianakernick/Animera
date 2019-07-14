//
//  tool widgets.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef tool_widgets_hpp
#define tool_widgets_hpp

#include "brush tool.hpp"
#include "select tools.hpp"
#include "transform tools.hpp"
#include "flood fill tool.hpp"
#include <QtWidgets/qwidget.h>
#include "drag paint tools.hpp"

/*class ToolWidget : public QWidget {
  Q_OBJECT
public:
  virtual std::string statusText() = 0;
};*/

class BrushToolWidget final : public QWidget {
public:
  using impl = BrushTool;
  static constexpr const char icon_name[] = "brush";
  static constexpr const char tooltip[] = "Brush Tool";
};

class FloodFillToolWidget final : public QWidget {
public:
  using impl = FloodFillTool;
  static constexpr const char icon_name[] = "flood fill";
  static constexpr const char tooltip[] = "Flood Fill Tool";
};

class RectangleSelectToolWidget final : public QWidget {
public:
  using impl = RectangleSelectTool;
  static constexpr const char icon_name[] = "rectangle select";
  static constexpr const char tooltip[] = "Rectangle Select Tool";
};

class PolygonSelectToolWidget final : public QWidget {
public:
  using impl = PolygonSelectTool;
  static constexpr const char icon_name[] = "polygon select";
  static constexpr const char tooltip[] = "Polygon Select Tool";
};

class WandSelectToolWidget final : public QWidget {
public:
  using impl = WandSelectTool;
  static constexpr const char icon_name[] = "wand select";
  static constexpr const char tooltip[] = "Wand Select Tool";
};

class LineToolWidget final : public QWidget {
public:
  using impl = LineTool;
  static constexpr const char icon_name[] = "line";
  static constexpr const char tooltip[] = "Line Tool";
};

class StrokedCircleToolWidget final : public QWidget {
public:
  using impl = StrokedCircleTool;
  static constexpr const char icon_name[] = "stroked circle";
  static constexpr const char tooltip[] = "Stroked Circle Tool";
};

class FilledCircleToolWidget final : public QWidget {
public:
  using impl = FilledCircleTool;
  static constexpr const char icon_name[] = "filled circle";
  static constexpr const char tooltip[] = "Filled Circle Tool";
};

class StrokedRectangleToolWidget final : public QWidget {
public:
  using impl = StrokedRectangleTool;
  static constexpr const char icon_name[] = "stroked rectangle";
  static constexpr const char tooltip[] = "Stroked Rectangle Tool";
};

class FilledRectangleToolWidget final : public QWidget {
public:
  using impl = FilledRectangleTool;
  static constexpr const char icon_name[] = "filled rectangle";
  static constexpr const char tooltip[] = "Filled Rectangle Tool";
};

class TranslateToolWidget final : public QWidget {
public:
  using impl = TranslateTool;
  static constexpr const char icon_name[] = "translate";
  static constexpr const char tooltip[] = "Translate Tool";
};

class FlipToolWidget final : public QWidget {
public:
  using impl = FlipTool;
  static constexpr const char icon_name[] = "flip";
  static constexpr const char tooltip[] = "Flip Tool";
};

class RotateToolWidget final : public QWidget {
public:
  using impl = RotateTool;
  static constexpr const char icon_name[] = "rotate";
  static constexpr const char tooltip[] = "Rotate Tool";
};

#endif
