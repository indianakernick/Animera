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

constexpr int min_thickness = 1;
constexpr int max_thickness = 64;
constexpr int no_radius = -1;
inline const QPoint no_point{-1, -1};
inline const QSize no_size{-1, -1};

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

  void setThickness(int);
  int getThickness() const;

private:
  QPen pen;
  
  void setColor(QColor);
  void setupPainter(QPainter &);
  void drawPoint(QPainter &, QPoint);
  void drawDrag(QPainter &, QPoint, QPoint);
  void drawOverlay(QImage *, QPoint);
};

enum class CircleCenter {
  c1x1,
  c1x2,
  c2x1,
  c2x2
};

class StrokedCircleTool final : public DragPaintTool<StrokedCircleTool> {
public:
  friend class DragPaintTool;

  StrokedCircleTool();
  ~StrokedCircleTool();

  void setThickness(int);
  int getThickness() const;
  void setCenter(CircleCenter);
  CircleCenter getCenter() const;
  int getRadius() const;
  
private:
  QPen pen;
  CircleCenter center = CircleCenter::c1x1;
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
  
  void setCenter(CircleCenter);
  CircleCenter getCenter() const;
  int getRadius() const;

private:
  CircleCenter center = CircleCenter::c1x1;
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
  
  void setThickness(int);
  int getThickness() const;
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
