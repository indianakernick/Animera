//
//  transform tools.hpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef transform_tools_hpp
#define transform_tools_hpp

#include "tool.hpp"

class TranslateTool final : public Tool {
public:
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseDownEvent &) override;
  void mouseMove(const ToolMouseMoveEvent &) override;
  void mouseUp(const ToolMouseUpEvent &) override;
  void keyPress(const ToolKeyEvent &) override;

private:
  bool drag = false;
  
  void translate(QPoint);
  void updateStatus();
};

class FlipTool final : public Tool {
public:
  void attachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseMove(const ToolMouseMoveEvent &) override;
  void keyPress(const ToolKeyEvent &) override;
  
private:
  bool flipX = false;
  bool flipY = false;
  
  void updateStatus();
};

class RotateTool final : public Tool {
public:
  void attachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseMove(const ToolMouseMoveEvent &) override;
  void keyPress(const ToolKeyEvent &) override;

private:
  int angle = 0;
  
  void updateStatus();
};

#endif
