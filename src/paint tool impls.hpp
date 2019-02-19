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

// @TODO make this a CRTP base
class DragPaintTool : public Tool {
public:
  DragPaintTool() = default;
  
  bool attachCell(Cell *) override final;
  ToolChanges mouseDown(const ToolEvent &) override final;
  ToolChanges mouseMove(const ToolEvent &) override final;
  ToolChanges mouseUp(const ToolEvent &) override final;
  
private:
  QPoint startPos = {-1, -1};
  QPoint lastPos = {-1, -1};
  ButtonType button = ButtonType::none;
  SourceCell *source = nullptr;
  QImage cleanImage;
  
  virtual void setColor(QColor) = 0;
  virtual void setupPainter(QPainter &) = 0;
  virtual void drawPoint(QPainter &, QPoint) = 0;
  virtual void drawDrag(QPainter &, QPoint, QPoint) = 0;
  virtual void drawOverlay(QImage *, QPoint) = 0;
};

class LineTool final : public DragPaintTool {
public:
  LineTool();

  void setThickness(int);
  int getThickness() const;
  
  static constexpr int min_thickness = 1;
  static constexpr int max_thickness = 64;

private:
  QPen pen;
  
  void setColor(QColor) override;
  void setupPainter(QPainter &) override;
  void drawPoint(QPainter &, QPoint) override;
  void drawDrag(QPainter &, QPoint, QPoint) override;
  void drawOverlay(QImage *, QPoint) override;
};

enum class CircleCenter {
  c1x1,
  c1x2,
  c2x1,
  c2x2
};

class StrokedCircleTool final : public DragPaintTool {
public:
  StrokedCircleTool();

  void setThickness(int);
  int getThickness() const;
  void setCenter(CircleCenter);
  CircleCenter getCenter() const;
  
  static constexpr int min_thickness = 1;
  static constexpr int max_thickness = 64;
  
private:
  QPen pen;
  CircleCenter center = CircleCenter::c1x1;

  void setColor(QColor) override;
  void setupPainter(QPainter &) override;
  void drawPoint(QPainter &, QPoint) override;
  void drawDrag(QPainter &, QPoint, QPoint) override;
  void drawOverlay(QImage *, QPoint) override;
};

#endif
