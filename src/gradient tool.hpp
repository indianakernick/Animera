//
//  gradient tool.hpp
//  Animera
//
//  Created by Indiana Kernick on 6/10/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_gradient_tool_hpp
#define animera_gradient_tool_hpp

#include "tool.hpp"
#include "cel.hpp"

class LinearGradientTool final : public Tool {
public:
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseDownEvent &) override;
  void mouseMove(const ToolMouseMoveEvent &) override;
  void mouseUp(const ToolMouseUpEvent &) override;

private:
  QPoint startPos;
  Cel cleanCel;
  LineGradMode mode = LineGradMode::hori;
  
  void drawGradient(QRect, QPoint);
};

#endif
