//
//  tool impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool impls.hpp"

#include <cmath>
#include "painting.hpp"
#include "composite.hpp"
#include "cell impls.hpp"

namespace {

QRgb selectColor(const ToolColors &colors, const ButtonType button) {
  switch (button) {
    case ButtonType::primary:   return colors.primary;
    case ButtonType::secondary: return colors.secondary;
    case ButtonType::erase:     return colors.erase;
    default: Q_UNREACHABLE();
  }
}

ToolChanges drawnChanges(const bool drawn) {
  return drawn ? ToolChanges::cell_overlay : ToolChanges::overlay;
}

}

bool BrushTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void BrushTool::detachCell() {
  assert(source);
  source = nullptr;
}

ToolChanges BrushTool::mouseDown(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  symPoint(*event.overlay, overlay_color, event.pos);
  lastPos = event.pos;
  color = selectColor(event.colors, event.button);
  return drawnChanges(symPoint(source->image.data, color, lastPos));
}

ToolChanges BrushTool::mouseMove(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  symPoint(*event.overlay, overlay_color, event.pos);
  if (event.button == ButtonType::none) return ToolChanges::overlay;
  Image &img = source->image;
  const bool drawn = symLine(img.data, color, {lastPos, event.pos});
  lastPos = event.pos;
  return drawnChanges(drawn);
}

ToolChanges BrushTool::mouseUp(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  Image &img = source->image;
  return drawnChanges(symLine(img.data, color, {lastPos, event.pos}));
}

void BrushTool::setWidth(const int newWidth) {
  assert(min_thickness <= newWidth && newWidth <= max_thickness);
  width = newWidth;
}

void BrushTool::setMode(const SymmetryMode newMode) {
  mode = newMode;
}

bool BrushTool::symPoint(QImage &img, const QRgb col, const QPoint point) {
  const QPoint refl = {img.width() - point.x() - 1, img.height() - point.y() - 1};
  bool drawn = drawRoundPoint(img, col, point, width);
  if (mode & SymmetryMode::hori) {
    drawn |= drawRoundPoint(img, col, {refl.x(), point.y()}, width);
  }
  if (mode & SymmetryMode::vert) {
    drawn |= drawRoundPoint(img, col, {point.x(), refl.y()}, width);
  }
  if (mode & SymmetryMode::both) {
    drawn |= drawRoundPoint(img, col, refl, width);
  }
  return drawn;
}

namespace {

QPoint reflectX(const QSize size, const QPoint point) {
  return {size.width() - point.x() - 1, point.y()};
}

QPoint reflectY(const QSize size, const QPoint point) {
  return {point.x(), size.height() - point.y() - 1};
}

QPoint reflectXY(const QSize size, const QPoint point) {
  return reflectX(size, reflectY(size, point));
}

}

bool BrushTool::symLine(QImage &img, const QRgb col, const QLine line) {
  const QSize size = img.size();
  bool drawn = drawRoundLine(img, col, line, width);
  if (mode & SymmetryMode::hori) {
    const QLine refl = {reflectX(size, line.p1()), reflectX(size, line.p2())};
    drawn |= drawRoundLine(img, col, refl, width);
  }
  if (mode & SymmetryMode::vert) {
    const QLine refl = {reflectY(size, line.p1()), reflectY(size, line.p2())};
    drawn |= drawRoundLine(img, col, refl, width);
  }
  if (mode & SymmetryMode::both) {
    const QLine refl = {reflectXY(size, line.p1()), reflectXY(size, line.p2())};
    drawn |= drawRoundLine(img, col, refl, width);
  }
  return drawn;
}

bool FloodFillTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void FloodFillTool::detachCell() {
  assert(source);
  source = nullptr;
}

ToolChanges FloodFillTool::mouseDown(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, overlay_color, event.pos);
  const QRgb color = selectColor(event.colors, event.button);
  return drawnChanges(drawFloodFill(source->image.data, color, event.pos));
}

ToolChanges FloodFillTool::mouseMove(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, overlay_color, event.pos);
  return ToolChanges::overlay;
}

ToolChanges FloodFillTool::mouseUp(const ToolEvent &) {
  assert(source);
  return ToolChanges::none;
}

bool RectangleSelectTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void RectangleSelectTool::detachCell() {
  assert(source);
  source = nullptr;
}

// @TODO maybe dragging a rectangle with secondary will cut instead of copy?

ToolChanges RectangleSelectTool::mouseDown(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, overlay_color, event.pos);
  } else { // SelectMode::paste
    blitImage(*event.overlay, overlay, event.pos + offset);
  }
  if (event.button != ButtonType::primary) return ToolChanges::overlay;
  if (mode == SelectMode::copy) {
    startPos = event.pos;
    return ToolChanges::overlay;
  } else { // SelectMode::paste
    blitImage(source->image.data, selection, event.pos + offset);
    return ToolChanges::cell_overlay;
  }
}

ToolChanges RectangleSelectTool::mouseMove(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    const QRect rect = QRect{startPos, event.pos}.normalized();
    drawStrokedRect(*event.overlay, overlay_color, rect);
  } else { // SelectMode::paste
    blitImage(*event.overlay, overlay, event.pos + offset);
  }
  return ToolChanges::overlay;
}

ToolChanges RectangleSelectTool::mouseUp(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, overlay_color, event.pos);
    const QRect rect = QRect{startPos, event.pos}.normalized();
    selection = blitImage(source->image.data, rect);
    overlay = selection;
    colorToOverlay(overlay);
    offset = startPos - event.pos;
    return ToolChanges::overlay;
  } else { // SelectMode::paste
    blitImage(*event.overlay, overlay, startPos);
    return ToolChanges::overlay;
  }
}

void RectangleSelectTool::setMode(const SelectMode newMode) {
  assert(button == ButtonType::none);
  mode = newMode;
}

template <typename Derived>
DragPaintTool<Derived>::~DragPaintTool() {
  static_assert(std::is_base_of_v<DragPaintTool, Derived>);
}

template <typename Derived>
bool DragPaintTool<Derived>::attachCell(Cell *cell) {
  source = dynamic_cast<SourceCell *>(cell);
  if (source) {
    if (!compatible(cleanImage, source->image.data)) {
      cleanImage = makeCompatible(source->image.data);
    }
    return true;
  } else {
    return false;
  }
}

template <typename Derived>
void DragPaintTool<Derived>::detachCell() {
  assert(source);
  source = nullptr;
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseDown(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  startPos = event.pos;
  copyImage(cleanImage, source->image.data);
  color = selectColor(event.colors, event.button);
  return drawnChanges(that()->drawPoint(source->image, startPos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseMove(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  if (event.button == ButtonType::none) return ToolChanges::overlay;
  copyImage(source->image.data, cleanImage);
  return drawnChanges(that()->drawDrag(source->image, startPos, event.pos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseUp(const ToolEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  copyImage(source->image.data, cleanImage);
  const bool drawn = that()->drawDrag(source->image, startPos, event.pos);
  startPos = no_point;
  return drawnChanges(drawn);
}

template <typename Derived>
bool DragPaintTool<Derived>::isDragging() const {
  return startPos != no_point;
}

template <typename Derived>
QRgb DragPaintTool<Derived>::getColor() const {
  return color;
}

template <typename Derived>
Derived *DragPaintTool<Derived>::that() {
  return static_cast<Derived *>(this);
}

LineTool::~LineTool() = default;

bool LineTool::drawPoint(Image &image, const QPoint pos) {
  return drawSquarePoint(image.data, getColor(), pos);
}

bool LineTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  return drawLine(image.data, getColor(), {start, end});
}

void LineTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, overlay_color, pos);
}

StrokedCircleTool::~StrokedCircleTool() = default;

void StrokedCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
}

int StrokedCircleTool::getRadius() const {
  return isDragging() ? radius : no_radius;
}

bool StrokedCircleTool::drawPoint(Image &, QPoint) {
  radius = 0;
  return false;
}

namespace {

double distance(const QPoint a, const QPoint b) {
  const int dx = a.x() - b.x();
  const int dy = a.y() - b.y();
  return std::sqrt(dx*dx + dy*dy);
}

int calcRadius(const QPoint start, const QPoint end) {
  return std::round(distance(start, end));
}

}

bool StrokedCircleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  radius = calcRadius(start, end);
  return drawStrokedEllipse(image.data, getColor(), circleToRect(start, radius, shape));
}

void StrokedCircleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawFilledRect(overlay, overlay_color, centerToRect(pos, shape));
}

FilledCircleTool::~FilledCircleTool() = default;

void FilledCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
}

int FilledCircleTool::getRadius() const {
  return isDragging() ? radius : no_radius;
}

bool FilledCircleTool::drawPoint(Image &, QPoint) {
  radius = 0;
  return false;
}

bool FilledCircleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  radius = calcRadius(start, end);
  return drawFilledEllipse(image.data, getColor(), circleToRect(start, radius, shape));
}

void FilledCircleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawFilledRect(overlay, overlay_color, centerToRect(pos, shape));
}

StrokedRectangleTool::~StrokedRectangleTool() = default;

QSize StrokedRectangleTool::getSize() const {
  return isDragging() ? size : no_size;
}

bool StrokedRectangleTool::drawPoint(Image &image, const QPoint pos) {
  size = QSize{1, 1};
  return drawSquarePoint(image.data, getColor(), pos);
}

bool StrokedRectangleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  size = rect.size();
  return drawStrokedRect(image.data, getColor(), rect);
}

void StrokedRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, overlay_color, pos);
}

FilledRectangleTool::~FilledRectangleTool() = default;

QSize FilledRectangleTool::getSize() const {
  return isDragging() ? size : no_size;
}

bool FilledRectangleTool::drawPoint(Image &image, const QPoint pos) {
  size = QSize{1, 1};
  return drawSquarePoint(image.data, getColor(), pos);
}

bool FilledRectangleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  size = rect.size();
  return drawFilledRect(image.data, getColor(), rect);
}

void FilledRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, overlay_color, pos);
}
