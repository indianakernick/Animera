//
//  brush tool.hpp
//  Animera
//
//  Created by Indiana Kernick on 14/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef brush_tool_hpp
#define brush_tool_hpp

#include "tool.hpp"

class BrushTool final : public Tool {
public:
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseDownEvent &) override;
  void mouseMove(const ToolMouseMoveEvent &) override;
  void mouseUp(const ToolMouseUpEvent &) override;

  void setRadius(int);
  void setMode(SymmetryMode);

private:
  int radius = brsh_radius.def;
  SymmetryMode mode = SymmetryMode::none;
  QRgb color = 0;
  QRect bounds;
  
  template <typename Func>
  void visit(Func, bool = true) const;
  template <typename Func>
  void visit(QPoint, Func, bool = true) const;
  template <typename Func>
  void visit(QLine, Func, bool = true) const;
  
  void symPointStatus(QPoint);
  void symPointOverlay(QPoint, QRgb);
  void symChangeOverlay(QLine);
  void symPoint(QPoint);
  void symLine(QLine);
  QRect symPointRect(QPoint) const;
  QRect pointRect(QPoint) const;
  QRect lineRect(QLine) const;
};

#endif
