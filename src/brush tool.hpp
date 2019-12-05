//
//  brush tool.hpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef brush_tool_hpp
#define brush_tool_hpp

#include "tool.hpp"

class BrushTool final : public Tool {
public:
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void mouseUp(const ToolMouseEvent &) override;

  void setRadius(int);
  void setMode(SymmetryMode);

private:
  // TODO: restore this to 0
  int radius = 4;
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
