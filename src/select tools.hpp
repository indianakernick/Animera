//
//  select tools.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef select_tools_hpp
#define select_tools_hpp

#include "tool.hpp"
#include "polygon.hpp"

class RectangleSelectTool final : public Tool {
public:
  void detachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void mouseUp(const ToolMouseEvent &) override;
  
private:
  QPoint startPos = no_point;
  QImage selection;
  QImage overlay;
  QPoint offset;
  SelectMode mode = SelectMode::copy;
};

// @TODO is polygon select really all that useful?
class PolygonSelectTool final : public Tool {
public:
  void detachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void mouseUp(const ToolMouseEvent &) override;
  
private:
  Polygon polygon;
  QImage selection;
  QImage mask;
  QImage overlay;
  QPoint offset;
  SelectMode mode = SelectMode::copy;
};

// @TODO What if you could remove from the selection by pressing undo?

class WandSelectTool final : public Tool {
public:
  void attachCell() override;
  void detachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void mouseUp(const ToolMouseEvent &) override;

private:
  QImage selection;
  QImage mask;
  QImage overlay;
  QPoint offset;
  SelectMode mode = SelectMode::copy;
  
  void toggleMode(const ToolMouseEvent &);
  void addToSelection(const ToolMouseEvent &);
};

#endif
