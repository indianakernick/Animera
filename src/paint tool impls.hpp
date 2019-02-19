//
//  paint tool impls.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef paint_tool_impls_hpp
#define paint_tool_impls_hpp

#include "tool.hpp"
#include <QtGui/qpen.h>

class SourceCell;

class BrushTool : public Tool {
public:
  BrushTool();

  bool attachCell(Cell *) override;
  ToolChanges mouseDown(const ToolEvent &) override;
  ToolChanges mouseMove(const ToolEvent &) override;
  ToolChanges mouseUp(const ToolEvent &) override;

  void setDiameter(int);
  int getDiameter() const;
  
  static constexpr int min_diameter = 1;
  static constexpr int max_diameter = 64;

private:
  QPoint lastPos = {-1, -1};
  SourceCell *source = nullptr;
  ButtonType button = ButtonType::none;
  QPen pen;
};

class LineTool : public Tool {
public:
  LineTool();
  
  bool attachCell(Cell *) override;
  ToolChanges mouseDown(const ToolEvent &) override;
  ToolChanges mouseMove(const ToolEvent &) override;
  ToolChanges mouseUp(const ToolEvent &) override;
  
  void setThickness(int);
  int getThickness() const;
  
  static constexpr int min_thickness = 1;
  static constexpr int max_thickness = 64;
  
private:
  QPoint lastPos = {-1, -1};
  QPoint startPos = {-1, -1};
  SourceCell *source = nullptr;
  ButtonType button = ButtonType::none;
  QPen pen;
  QImage cleanImage;
};

enum class CircleCenter {
  c1x1,
  c1x2,
  c2x1,
  c2x2
};

class StrokedCircleTool : public Tool {
public:
  StrokedCircleTool();
  
  bool attachCell(Cell *) override;
  ToolChanges mouseDown(const ToolEvent &) override;
  ToolChanges mouseMove(const ToolEvent &) override;
  ToolChanges mouseUp(const ToolEvent &) override;
  
  void setThickness(int);
  int getThickness() const;
  void setCenter(CircleCenter);
  CircleCenter getCenter() const;
  
  static constexpr int min_thickness = 1;
  static constexpr int max_thickness = 64;
  
private:
  QPoint lastPos = {-1, -1};
  QPoint startPos = {-1, -1};
  SourceCell *source = nullptr;
  ButtonType button = ButtonType::none;
  QPen pen;
  QImage cleanImage;
  CircleCenter center = CircleCenter::c1x1;
};

#endif
