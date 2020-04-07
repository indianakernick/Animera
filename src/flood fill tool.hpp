//
//  flood fill tool.hpp
//  Animera
//
//  Created by Indiana Kernick on 14/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_flood_fill_tool_hpp
#define animera_flood_fill_tool_hpp

#include "tool.hpp"

class FloodFillTool final : public Tool {
public:
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseDownEvent &) override;
  void mouseMove(const ToolMouseMoveEvent &) override;

private:
  template <typename Pixel>
  QRect fill(QRect, QPoint, QRgb);
  QRect fill(QRect, QPoint, QRgb);
  void fillOpen(QPoint, QRgb);
};

#endif
