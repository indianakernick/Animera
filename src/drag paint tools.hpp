//
//  drag paint tools.hpp
//  Animera
//
//  Created by Indiana Kernick on 14/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_drag_paint_tools_hpp
#define animera_drag_paint_tools_hpp

#include "tool.hpp"
#include "cell.hpp"

template <typename Derived>
class DragPaintTool : public Tool {
public:
  ~DragPaintTool();
  
  void attachCell() override final;
  void detachCell() override final;
  void mouseLeave(const ToolLeaveEvent &) override final;
  void mouseDown(const ToolMouseDownEvent &) override final;
  void mouseMove(const ToolMouseMoveEvent &) override final;
  void mouseUp(const ToolMouseUpEvent &) override final;

protected:
  QRgb getColor() const;

private:
  QPoint startPos = no_point;
  Cell cleanCell;
  QRgb color = 0;
  
  Derived *that();
};

class LineTool final : public DragPaintTool<LineTool> {
public:
  friend class DragPaintTool;

  ~LineTool();
  
  void setRadius(int);

private:
  int radius = line_radius.def;
 
  bool drawPoint(QImage &, QRgb, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

class StrokedCircleTool final : public DragPaintTool<StrokedCircleTool> {
public:
  friend class DragPaintTool;

  ~StrokedCircleTool();

  void setShape(gfx::CircleShape);
  void setThick(int);
  
private:
  gfx::CircleShape shape = gfx::CircleShape::c1x1;
  int thickness = circ_thick.def;

  bool drawPoint(QImage &, QRgb, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

class FilledCircleTool final : public DragPaintTool<FilledCircleTool> {
public:
  friend class DragPaintTool;
  
  ~FilledCircleTool();
  
  void setShape(gfx::CircleShape);

private:
  gfx::CircleShape shape = gfx::CircleShape::c1x1;
  
  bool drawPoint(QImage &, QRgb, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

class StrokedRectangleTool final : public DragPaintTool<StrokedRectangleTool> {
public:
  friend class DragPaintTool;
  
  ~StrokedRectangleTool();
  
  void setThick(int);

private:
  int thickness = rect_thick.def;

  bool drawPoint(QImage &, QRgb, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

class FilledRectangleTool final : public DragPaintTool<FilledRectangleTool> {
public:
  friend class DragPaintTool;
  
  ~FilledRectangleTool();

private:
  bool drawPoint(QImage &, QRgb, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

#endif
