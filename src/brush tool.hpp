//
//  brush tool.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef brush_tool_hpp
#define brush_tool_hpp

#include "tool.hpp"

class BrushTool final : public Tool {
public:
  void detachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void mouseUp(const ToolMouseEvent &) override;

  void setRadius(int);
  void setMode(SymmetryMode);

private:
  QPoint lastPos = no_point;
  // @TODO restore these to 0 and none
  int radius = 0;
  SymmetryMode mode = SymmetryMode::none;
  QRgb color = 0;
  
  void symPointStatus(QPoint);
  bool symPoint(QImage &, QRgb, QPoint);
  bool symLine(QImage &, QRgb, QLine);
};

#endif
