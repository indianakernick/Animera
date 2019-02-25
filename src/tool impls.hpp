//
//  tool impls.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef tool_impls_hpp
#define tool_impls_hpp

#include "tool.hpp"
#include "paint params.hpp"

class SourceCell;

class BrushTool final : public Tool {
public:
  bool attachCell(Cell *) override;
  void detachCell() override;
  ToolChanges mouseDown(const ToolEvent &) override;
  ToolChanges mouseMove(const ToolEvent &) override;
  ToolChanges mouseUp(const ToolEvent &) override;

  void setWidth(int);
  void setMode(SymmetryMode);

private:
  QPoint lastPos = no_point;
  SourceCell *source = nullptr;
  ButtonType button = ButtonType::none;
  int width = 1;
  SymmetryMode mode = SymmetryMode::none;
  QRgb color = 0;
  
  bool symPoint(QImage &, QRgb, QPoint);
  bool symLine(QImage &, QRgb, QLine);
};

class FloodFillTool final : public Tool {
public:
  bool attachCell(Cell *) override;
  void detachCell() override;
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
  void detachCell() override final;
  ToolChanges mouseDown(const ToolEvent &) override final;
  ToolChanges mouseMove(const ToolEvent &) override final;
  ToolChanges mouseUp(const ToolEvent &) override final;

protected:
  bool isDragging() const;
  QRgb getColor() const;

private:
  QPoint startPos = no_point;
  QPoint lastPos = no_point;
  ButtonType button = ButtonType::none;
  SourceCell *source = nullptr;
  QImage cleanImage;
  QRgb color = 0;
  
  Derived *that();
};

class LineTool final : public DragPaintTool<LineTool> {
public:
  friend class DragPaintTool;

  ~LineTool();

private:
  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

class StrokedCircleTool final : public DragPaintTool<StrokedCircleTool> {
public:
  friend class DragPaintTool;

  ~StrokedCircleTool();

  void setShape(CircleShape);
  int getRadius() const;
  
private:
  CircleShape shape = CircleShape::c1x1;
  int radius = no_radius;

  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

class FilledCircleTool final : public DragPaintTool<FilledCircleTool> {
public:
  friend class DragPaintTool;
  
  ~FilledCircleTool();
  
  void setShape(CircleShape);
  int getRadius() const;

private:
  CircleShape shape = CircleShape::c1x1;
  int radius = no_radius;
  
  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

class StrokedRectangleTool final : public DragPaintTool<StrokedRectangleTool> {
public:
  friend class DragPaintTool;
  
  ~StrokedRectangleTool();
  
  QSize getSize() const;

private:
  QSize size = no_size;
  
  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

class FilledRectangleTool final : public DragPaintTool<FilledRectangleTool> {
public:
  friend class DragPaintTool;
  
  ~FilledRectangleTool();

  QSize getSize() const;

private:
  QSize size = no_size;
  
  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

#endif
