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
#include "paint params.hpp"

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

private:
  QPoint lastPos = no_point;
  SourceCell *source = nullptr;
  ButtonType button = ButtonType::none;
  QPen pen;
};

class FloodFillTool : public Tool {
public:
  bool attachCell(Cell *) override;
  ToolChanges mouseDown(const ToolEvent &) override;
  ToolChanges mouseMove(const ToolEvent &) override;
  ToolChanges mouseUp(const ToolEvent &) override;

private:
  QPoint lastPos = no_point;
  SourceCell *source = nullptr;
};

template <typename Derived>
class DragPaintTool : public Tool {
public:
  ~DragPaintTool();
  
  bool attachCell(Cell *) override final;
  ToolChanges mouseDown(const ToolEvent &) override final;
  ToolChanges mouseMove(const ToolEvent &) override final;
  ToolChanges mouseUp(const ToolEvent &) override final;

protected:
  bool isDragging() const;

private:
  QPoint startPos = no_point;
  QPoint lastPos = no_point;
  ButtonType button = ButtonType::none;
  SourceCell *source = nullptr;
  QImage cleanImage;
  
  Derived *that();
};

class LineTool final : public DragPaintTool<LineTool> {
public:
  friend class DragPaintTool;

  LineTool();
  ~LineTool();

private:
  QPen pen;
  
  void setColor(QColor);
  void setupPainter(QPainter &);
  void drawPoint(QPainter &, QPoint);
  void drawDrag(QPainter &, QPoint, QPoint);
  void drawOverlay(QImage *, QPoint);
};

class StrokedCircleTool final : public DragPaintTool<StrokedCircleTool> {
public:
  friend class DragPaintTool;

  StrokedCircleTool();
  ~StrokedCircleTool();

  void setShape(CircleShape);
  CircleShape getShape() const;
  int getRadius() const;
  
private:
  QPen pen;
  CircleShape shape = CircleShape::c1x1;
  int radius = no_radius;

  void setColor(QColor);
  void setupPainter(QPainter &);
  void drawPoint(QPainter &, QPoint);
  void drawDrag(QPainter &, QPoint, QPoint);
  void drawOverlay(QImage *, QPoint);
};

class FilledCircleTool final : public DragPaintTool<FilledCircleTool> {
public:
  friend class DragPaintTool;
  
  FilledCircleTool();
  ~FilledCircleTool();
  
  void setShape(CircleShape);
  CircleShape getShape() const;
  int getRadius() const;

private:
  CircleShape shape = CircleShape::c1x1;
  QPen pen;
  int radius = no_radius;
  
  void setColor(QColor);
  void setupPainter(QPainter &);
  void drawPoint(QPainter &, QPoint);
  void drawDrag(QPainter &, QPoint, QPoint);
  void drawOverlay(QImage *, QPoint);
};

class StrokedRectangleTool final : public DragPaintTool<StrokedRectangleTool> {
public:
  friend class DragPaintTool;
  
  StrokedRectangleTool();
  ~StrokedRectangleTool();
  
  QSize getSize() const;

private:
  QPen pen;
  QSize size = no_size;
  
  void setColor(QColor);
  void setupPainter(QPainter &);
  void drawPoint(QPainter &, QPoint);
  void drawDrag(QPainter &, QPoint, QPoint);
  void drawOverlay(QImage *, QPoint);
};

class FilledRectangleTool final : public DragPaintTool<FilledRectangleTool> {
public:
  friend class DragPaintTool;
  
  FilledRectangleTool();
  ~FilledRectangleTool();

  QSize getSize() const;

private:
  QPen pen;
  QSize size = no_size;
  
  void setColor(QColor);
  void setupPainter(QPainter &);
  void drawPoint(QPainter &, QPoint);
  void drawDrag(QPainter &, QPoint, QPoint);
  void drawOverlay(QImage *, QPoint);
};

#endif
