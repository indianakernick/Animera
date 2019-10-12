//
//  drag paint tools.hpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef drag_paint_tools_hpp
#define drag_paint_tools_hpp

#include "tool.hpp"
#include "cell.hpp"

template <typename Derived>
class DragPaintTool : public Tool {
public:
  ~DragPaintTool();
  
  void attachCell() override final;
  void detachCell() override final;
  void mouseLeave(const ToolLeaveEvent &) override final;
  void mouseDown(const ToolMouseEvent &) override final;
  void mouseMove(const ToolMouseEvent &) override final;
  void mouseUp(const ToolMouseEvent &) override final;

protected:
  bool isDragging() const;
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
  // TODO: restore this to 0
  int radius = 2;
 
  bool drawPoint(QImage &, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
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
  // TODO: restore this to 1
  int thickness = 8;

  bool drawPoint(QImage &, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
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
  
  bool drawPoint(QImage &, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
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
  // TODO: restore this to 1
  int thickness = 4;

  bool drawPoint(QImage &, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

class FilledRectangleTool final : public DragPaintTool<FilledRectangleTool> {
public:
  friend class DragPaintTool;
  
  ~FilledRectangleTool();

private:
  bool drawPoint(QImage &, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

#endif
