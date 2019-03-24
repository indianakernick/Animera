//
//  tool widgets.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef tool_widgets_hpp
#define tool_widgets_hpp

#include "tool impls.hpp"
#include <QtWidgets/qwidget.h>

/*class ToolWidget : public QWidget {
  Q_OBJECT
public:
  virtual std::string statusText() = 0;
};*/

class BrushToolWidget final : public QWidget {
public:
  using impl = BrushTool;
  static constexpr const char icon_name[] = "brush";
  static constexpr const char tooltip[] = "Brush Tool\n\nPress V to clear the frame";
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
  static constexpr const char tooltip[] = "Rectangle Select Tool\n\nPress P to toggle between COPY and PASTE modes";
};

class MaskSelectToolWidget final : public QWidget {
public:
  using impl = MaskSelectTool;
  static constexpr const char icon_name[] = "mask select";
  static constexpr const char tooltip[] = "Mask Select Tool\n\nPress P to toggle between COPY and PASTE modes";
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
  static constexpr const char tooltip[] = "Translate Tool\n\nUse arrow keys or drag";
};

class FlipToolWidget final : public QWidget {
public:
  using impl = FlipTool;
  static constexpr const char icon_name[] = "flip";
  static constexpr const char tooltip[] = "Flip Tool\n\nUse arrow keys";
};

class RotateToolWidget final : public QWidget {
public:
  using impl = RotateTool;
  static constexpr const char icon_name[] = "rotate";
  static constexpr const char tooltip[] = "Rotate Tool\n\nUse arrow keys";
};

#endif
