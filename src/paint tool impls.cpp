//
//  paint tool impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "paint tool impls.hpp"

#include <cmath>
#include "painting.hpp"
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
  assert(event.overlay);
  if (button != ButtonType::none) return ToolChanges::none;
  clearImage(*event.overlay);
  drawRoundPoint(*event.overlay, overlay_color, event.pos, width);
  button = event.type;
  lastPos = event.pos;
  color = selectColor(event.colors, event.type);
  Image &img = source->image;
  return drawnChanges(drawRoundPoint(img.data, color, lastPos, width));
}

ToolChanges BrushTool::mouseMove(const ToolEvent &event) {
  assert(source);
  assert(event.overlay);
  if (event.pos == lastPos) return ToolChanges::none;
  clearImage(*event.overlay);
  drawRoundPoint(*event.overlay, overlay_color, event.pos, width);
  if (event.type == ButtonType::none) return ToolChanges::overlay;
  Image &img = source->image;
  const bool drawn = drawRoundLine(img.data, color, {lastPos, event.pos}, width);
  lastPos = event.pos;
  return drawnChanges(drawn);
}

ToolChanges BrushTool::mouseUp(const ToolEvent &event) {
  assert(source);
  assert(event.overlay);
  if (event.pos == lastPos) return ToolChanges::none;
  if (event.type != button) return ToolChanges::none;
  clearImage(*event.overlay);
  button = ButtonType::none;
  Image &img = source->image;
  return drawnChanges(drawRoundLine(img.data, color, {lastPos, event.pos}, width));
}

void BrushTool::setWidth(const int newWidth) {
  assert(min_thickness <= newWidth && newWidth <= max_thickness);
  width = newWidth;
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
  assert(event.overlay);
  lastPos = event.pos;
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, overlay_color, event.pos);
  const QRgb color = selectColor(event.colors, event.type);
  Image &img = source->image;
  return drawnChanges(drawFloodFill(img.data, color, event.pos));
}

ToolChanges FloodFillTool::mouseMove(const ToolEvent &event) {
  assert(source);
  assert(event.overlay);
  if (event.pos == lastPos) return ToolChanges::none;
  lastPos = event.pos;
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, overlay_color, event.pos);
  return ToolChanges::overlay;
}

ToolChanges FloodFillTool::mouseUp(const ToolEvent &) {
  assert(source);
  return ToolChanges::none;
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
  assert(event.overlay);
  if (button != ButtonType::none) return ToolChanges::none;
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  button = event.type;
  startPos = lastPos = event.pos;
  copyImage(cleanImage, source->image.data);
  color = selectColor(event.colors, event.type);
  return drawnChanges(that()->drawPoint(source->image, startPos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseMove(const ToolEvent &event) {
  assert(source);
  assert(event.overlay);
  if (event.pos == lastPos) return ToolChanges::none;
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  if (event.type == ButtonType::none) return ToolChanges::overlay;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  return drawnChanges(that()->drawDrag(source->image, startPos, lastPos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseUp(const ToolEvent &event) {
  assert(source);
  assert(event.overlay);
  if (event.pos == lastPos) return ToolChanges::none;
  if (event.type != button) return ToolChanges::none;
  clearImage(*event.overlay);
  button = ButtonType::none;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  const bool drawn = that()->drawDrag(source->image, startPos, lastPos);
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
