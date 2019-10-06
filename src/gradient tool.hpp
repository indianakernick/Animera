//
//  gradient tool.hpp
//  Animera
//
//  Created by Indi Kernick on 6/10/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef gradient_tool_hpp
#define gradient_tool_hpp

#include "tool.hpp"
#include "cell.hpp"

class LinearGradientTool final : public Tool {
public:
  void detachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void mouseUp(const ToolMouseEvent &) override;

private:
  QPoint startPos;
  Cell cleanCell;
  LineGradMode mode = LineGradMode::hori;
  
  void updateStatus(const ToolMouseEvent &, QRect);
  void drawGradient(QRect, QPoint);
};

#endif
