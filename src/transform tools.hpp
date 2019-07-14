//
//  transform tools.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef transform_tools_hpp
#define transform_tools_hpp

#include "tool.hpp"

class TranslateTool final : public Tool {
public:
  void attachCell() override;
  void detachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void mouseUp(const ToolMouseEvent &) override;
  void keyPress(const ToolKeyEvent &) override;

private:
  QImage cleanImage;
  QPoint lastPos = no_point;
  QPoint pos = no_point;
  bool drag = false;
  
  void translate(QPoint, QRgb);
  void updateSourceImage(QRgb);
  void updateStatus();
};

class FlipTool final : public Tool {
public:
  void attachCell() override;
  void detachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void keyPress(const ToolKeyEvent &) override;
  
private:
  bool flipX = false;
  bool flipY = false;
  
  void updateStatus();
};

class RotateTool final : public Tool {
public:
  void attachCell() override;
  void detachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void keyPress(const ToolKeyEvent &) override;

private:
  int angle = 0;
  bool square;
  
  void updateStatus();
};

#endif
