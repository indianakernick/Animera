//
//  tool impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool impls.hpp"

#include <cmath>
#include "config.hpp"
#include "painting.hpp"
#include "composite.hpp"
#include "cell impls.hpp"

#include <iostream>

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

std::string &operator+=(std::string &status, const QPoint point) {
  status += '[';
  status += std::to_string(point.x());
  status += ' ';
  status += std::to_string(point.y());
  status += ']';
  return status;
}

std::string &operator+=(std::string &status, const QSize size) {
  return status += QPoint{size.width(), size.height()};
}

// @TODO should I put these functions in a class?

void statusPos(std::string &status, const QPoint pos) {
  status += "POS: ";
  status += pos;
}

void statusPosSize(std::string &status, const QPoint pos, const QSize size) {
  statusPos(status, pos);
  status += " SIZE: ";
  status += size;
}

void statusMode(std::string &status, const SelectMode mode) {
  if (mode == SelectMode::copy) {
    status += "COPY - ";
  } else if (mode == SelectMode::paste) {
    status += "PASTE - ";
  } else Q_UNREACHABLE();
}

void statusBool(std::string &status, const bool b) {
  status += b ? "YES" : "NO";
}

}

bool BrushTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void BrushTool::detachCell() {
  source = nullptr;
}

ToolChanges BrushTool::mouseDown(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  symPoint(*event.overlay, overlay_color, event.pos);
  symPoint(*event.status, event.pos);
  lastPos = event.pos;
  color = selectColor(event.colors, event.button);
  return drawnChanges(symPoint(source->image.data, color, lastPos));
}

ToolChanges BrushTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  symPoint(*event.overlay, overlay_color, event.pos);
  symPoint(*event.status, event.pos);
  if (event.button == ButtonType::none) return ToolChanges::overlay;
  Image &img = source->image;
  const bool drawn = symLine(img.data, color, {lastPos, event.pos});
  lastPos = event.pos;
  return drawnChanges(drawn);
}

ToolChanges BrushTool::mouseUp(const ToolMouseEvent &event) {
  symPoint(*event.status, event.pos);
  return drawnChanges(symLine(source->image.data, color, {lastPos, event.pos}));
}

ToolChanges BrushTool::keyPress(const ToolKeyEvent &event) {
  if (event.key == key_clear) {
    clearImage(source->image.data, event.colors.erase);
    return ToolChanges::cell;
  }
  return ToolChanges::none;
}

void BrushTool::setWidth(const int newWidth) {
  assert(min_thickness <= newWidth && newWidth <= max_thickness);
  width = newWidth;
}

void BrushTool::setMode(const SymmetryMode newMode) {
  mode = newMode;
}

void BrushTool::symPoint(std::string &status, const QPoint point) {
  const QSize size = source->image.data.size();
  const QPoint refl = {size.width() - point.x() - 1, size.height() - point.y() - 1};
  status += "POS: ";
  status += point;
  if (mode & SymmetryMode::hori) {
    status += ' ';
    status += QPoint{refl.x(), point.y()};
  }
  if (mode & SymmetryMode::vert) {
    status += ' ';
    status += QPoint{point.x(), refl.y()};
  }
  if (mode & SymmetryMode::both) {
    status += ' ';
    status += refl;
  }
}

bool BrushTool::symPoint(QImage &img, const QRgb col, const QPoint point) {
  // @TODO symmetry isn't quite right when brush size isn't 1
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

ToolChanges FloodFillTool::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, overlay_color, event.pos);
  statusPos(*event.status, event.pos);
  const QRgb color = selectColor(event.colors, event.button);
  return drawnChanges(drawFloodFill(source->image.data, color, event.pos));
}

ToolChanges FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, overlay_color, event.pos);
  statusPos(*event.status, event.pos);
  return ToolChanges::overlay;
}

bool RectangleSelectTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void RectangleSelectTool::detachCell() {
  assert(source);
  source = nullptr;
}

// @TODO maybe dragging a rectangle with secondary will cut instead of copy?

ToolChanges RectangleSelectTool::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  statusMode(*event.status, mode);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, overlay_color, event.pos);
    statusPos(*event.status, event.pos);
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    statusPosSize(*event.status, event.pos + offset, overlay.size());
  } else Q_UNREACHABLE();
  if (event.button != ButtonType::primary) return ToolChanges::overlay;
  if (mode == SelectMode::copy) {
    startPos = event.pos;
    return ToolChanges::overlay;
  } else if (mode == SelectMode::paste) {
    blitImage(source->image.data, selection, event.pos + offset);
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges RectangleSelectTool::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  statusMode(*event.status, mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      const QRect rect = QRect{startPos, event.pos}.normalized();
      drawStrokedRect(*event.overlay, overlay_color, rect);
      statusPosSize(*event.status, rect.topLeft(), rect.size());
    } else {
      drawSquarePoint(*event.overlay, overlay_color, event.pos);
      statusPos(*event.status, event.pos);
    }
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    statusPosSize(*event.status, event.pos + offset, overlay.size());
  } else Q_UNREACHABLE();
  return ToolChanges::overlay;
}

ToolChanges RectangleSelectTool::mouseUp(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, overlay_color, event.pos);
    const QRect rect = QRect{startPos, event.pos}.normalized();
    selection = blitImage(source->image.data, rect);
    overlay = selection;
    colorToOverlay(overlay);
    offset = rect.topLeft() - event.pos;
    mode = SelectMode::paste;
  }
  
  startPos = no_point;
  blitImage(*event.overlay, overlay, event.pos + offset);
  statusMode(*event.status, mode);
  statusPosSize(*event.status, event.pos + offset, overlay.size());
  return ToolChanges::overlay;
}

ToolChanges RectangleSelectTool::keyPress(const ToolKeyEvent &event) {
  if (event.key == key_toggle_copy_paste && startPos == no_point) {
    mode = opposite(mode);
  }
  statusMode(*event.status, mode);
  return ToolChanges::none;
}

void RectangleSelectTool::setMode(const SelectMode newMode) {
  mode = newMode;
}

bool MaskSelectTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void MaskSelectTool::detachCell() {
  assert(source);
  source = nullptr;
}

ToolChanges MaskSelectTool::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  statusMode(*event.status, mode);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, overlay_color, event.pos);
    statusPos(*event.status, event.pos);
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    statusPosSize(*event.status, event.pos + offset, overlay.size());
  } else Q_UNREACHABLE();
  if (event.button != ButtonType::primary) return ToolChanges::overlay;
  if (mode == SelectMode::copy) {
    polygon.init(event.pos);
    return ToolChanges::overlay;
  } else if (mode == SelectMode::paste) {
    blitMaskImage(source->image.data, mask, selection, event.pos + offset);
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges MaskSelectTool::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  statusMode(*event.status, mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      polygon.push(event.pos);
      drawFilledPolygon(*event.overlay, overlay_color, polygon, QPoint{0, 0});
      statusPosSize(*event.status, polygon.bounds().topLeft(), polygon.bounds().size());
    } else {
      drawSquarePoint(*event.overlay, overlay_color, event.pos);
      statusPos(*event.status, event.pos);
    }
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    statusPosSize(*event.status, event.pos + offset, overlay.size());
  } else Q_UNREACHABLE();
  return ToolChanges::overlay;
}

ToolChanges MaskSelectTool::mouseUp(const ToolMouseEvent &event) {
  QPolygon p;
  assert(source);
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    polygon.push(event.pos);
    const QRect clippedBounds = polygon.bounds().intersected(source->image.data.rect());
    mask = QImage{clippedBounds.size(), mask_format};
    clearImage(mask);
    drawFilledPolygon(mask, mask_color_on, polygon, -clippedBounds.topLeft());
    selection = blitMaskImage(source->image.data, mask, clippedBounds.topLeft());
    overlay = selection;
    colorToOverlay(overlay, mask);
    offset = clippedBounds.topLeft() - event.pos;
    mode = SelectMode::paste;
  }
  
  blitImage(*event.overlay, overlay, event.pos + offset);
  statusMode(*event.status, mode);
  statusPosSize(*event.status, event.pos + offset, overlay.size());
  return ToolChanges::overlay;
}

ToolChanges MaskSelectTool::keyPress(const ToolKeyEvent &event) {
  // @TODO make sure the mouse isn't down
  if (event.key == key_toggle_copy_paste) {
    mode = opposite(mode);
  }
  statusMode(*event.status, mode);
  return ToolChanges::none;
}

void MaskSelectTool::setMode(const SelectMode newMode) {
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
ToolChanges DragPaintTool<Derived>::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  that()->updateStatus(*event.status, event.pos, event.pos);
  startPos = event.pos;
  copyImage(cleanImage, source->image.data);
  color = selectColor(event.colors, event.button);
  return drawnChanges(that()->drawPoint(source->image, startPos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  if (event.button == ButtonType::none) {
    statusPos(*event.status, event.pos);
    return ToolChanges::overlay;
  }
  that()->updateStatus(*event.status, startPos, event.pos);
  copyImage(source->image.data, cleanImage);
  return drawnChanges(that()->drawDrag(source->image, startPos, event.pos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseUp(const ToolMouseEvent &event) {
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

void LineTool::updateStatus(std::string &status, const QPoint start, const QPoint end) {
  status += "START: ";
  status += start;
  status += " END: ";
  status += end;
}

StrokedCircleTool::~StrokedCircleTool() = default;

void StrokedCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
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

void StrokedCircleTool::updateStatus(std::string &status, const QPoint start, QPoint) {
  status += "CENTER: ";
  status += start;
  status += " RADIUS: ";
  status += std::to_string(radius);
}

FilledCircleTool::~FilledCircleTool() = default;

void FilledCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
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

void FilledCircleTool::updateStatus(std::string &status, const QPoint start, QPoint) {
  status += "CENTER: ";
  status += start;
  status += " RADIUS: ";
  status += std::to_string(radius);
}

StrokedRectangleTool::~StrokedRectangleTool() = default;

bool StrokedRectangleTool::drawPoint(Image &image, const QPoint pos) {
  return drawSquarePoint(image.data, getColor(), pos);
}

bool StrokedRectangleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  return drawStrokedRect(image.data, getColor(), rect);
}

void StrokedRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, overlay_color, pos);
}

void StrokedRectangleTool::updateStatus(std::string &status, const QPoint start, const QPoint end) {
  QRect rect = QRect{start, end}.normalized();
  statusPosSize(status, rect.topLeft(), rect.size());
}

FilledRectangleTool::~FilledRectangleTool() = default;

bool FilledRectangleTool::drawPoint(Image &image, const QPoint pos) {
  return drawSquarePoint(image.data, getColor(), pos);
}

bool FilledRectangleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  return drawFilledRect(image.data, getColor(), rect);
}

void FilledRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, overlay_color, pos);
}

void FilledRectangleTool::updateStatus(std::string &status, const QPoint start, const QPoint end) {
  QRect rect = QRect{start, end}.normalized();
  statusPosSize(status, rect.topLeft(), rect.size());
}

bool TranslateTool::attachCell(Cell *cell) {
  if ((source = dynamic_cast<SourceCell *>(cell))) {
    if (!compatible(cleanImage, source->image.data)) {
      cleanImage = makeCompatible(source->image.data);
    }
    copyImage(cleanImage, source->image.data);
    pos = {0, 0};
    return true;
  } else if ((transform = dynamic_cast<TransformCell *>(cell))) {
    return true;
  }
  return false;
}

namespace {

template <typename A, typename B>
bool oneNotNull(A *a, B *b) {
  return !!a + !!b == 1;
}

}

void TranslateTool::detachCell() {
  source = nullptr;
  transform = nullptr;
}

ToolChanges TranslateTool::mouseDown(const ToolMouseEvent &event) {
  assert(oneNotNull(source, transform));
  if (event.button != ButtonType::primary) return ToolChanges::none;
  lastPos = event.pos;
  drag = true;
  return ToolChanges::none;
}

ToolChanges TranslateTool::mouseMove(const ToolMouseEvent &event) {
  assert(oneNotNull(source, transform));
  if (event.button != ButtonType::primary || !drag) {
    updateStatus(*event.status);
    return ToolChanges::none;
  }
  translate(event.pos - lastPos, event.colors.erase);
  updateStatus(*event.status);
  lastPos = event.pos;
  return ToolChanges::cell;
}

ToolChanges TranslateTool::mouseUp(const ToolMouseEvent &event) {
  assert(oneNotNull(source, transform));
  if (event.button != ButtonType::primary || !drag) return ToolChanges::none;
  translate(event.pos - lastPos, event.colors.erase);
  updateStatus(*event.status);
  lastPos = event.pos;
  drag = false;
  return ToolChanges::cell;
}

namespace {

QPoint arrowToDir(const Qt::Key key) {
  switch (key) {
    case key_mov_up:    return {0, -1};
    case key_mov_right: return {1, 0};
    case key_mov_down:  return {0, 1};
    case key_mov_left:  return {-1, 0};
    default:            return {0, 0};
  }
}

}

ToolChanges TranslateTool::keyPress(const ToolKeyEvent &event) {
  assert(oneNotNull(source, transform));
  QPoint move = arrowToDir(event.key);
  if (move == QPoint{0, 0}) return ToolChanges::none;
  translate(move, event.colors.erase);
  updateStatus(*event.status);
  return ToolChanges::cell;
}

namespace {

Transform xformFromPos(const QPoint pos) {
  Transform xform;
  xform.posX = pos.x();
  xform.posY = pos.y();
  return xform;
}

}

void TranslateTool::translate(const QPoint move, const QRgb eraseColor) {
  assert(oneNotNull(source, transform));
  if (source) {
    pos += move;
    updateSourceImage(eraseColor);
  } else if (transform) {
    Transform &xform = transform->xform;
    xform.posX += move.x();
    xform.posY += move.y();
  } else Q_UNREACHABLE();
}

void TranslateTool::updateSourceImage(const QRgb eraseColor) {
  assert(source);
  assert(!transform);
  clearImage(source->image.data, eraseColor);
  Image src{cleanImage, xformFromPos(pos)};
  blitTransformedImage(source->image.data, src);
}

void TranslateTool::updateStatus(std::string &status) {
  if (source) {
    statusPos(status, pos);
  } else if (transform) {
    Transform &xform = transform->xform;
    statusPos(status, {xform.posX, xform.posY});
  } else Q_UNREACHABLE();
}

bool FlipTool::attachCell(Cell *cell) {
  if ((source = dynamic_cast<SourceCell *>(cell))) {
    return true;
  } else if ((transform = dynamic_cast<TransformCell *>(cell))) {
    return true;
  }
  return false;
}

void FlipTool::detachCell() {
  if (source) {
    updateSourceImage();
  }
}

namespace {

bool arrowToFlip(const Qt::Key key, Transform &xform) {
  switch (key) {
    // return true if changed
    case key_flp_on_x: return !std::exchange(xform.flipX, true);
    case key_flp_on_y: return !std::exchange(xform.flipY, true);
    case key_flp_off_y: return std::exchange(xform.flipY, false);
    case key_flp_off_x: return std::exchange(xform.flipX, false);
    default: return false;
  }
}

Transform &getTransform(SourceCell *source, TransformCell *transform) {
  assert(oneNotNull(source, transform));
  if (source) {
    return source->image.xform;
  } else if (transform) {
    return transform->xform;
  } else Q_UNREACHABLE();
}

}

ToolChanges FlipTool::mouseMove(const ToolMouseEvent &event) {
  updateStatus(*event.status);
  return ToolChanges::none;
}

ToolChanges FlipTool::keyPress(const ToolKeyEvent &event) {
  assert(oneNotNull(source, transform));
  Transform &xform = getTransform(source, transform);
  if (arrowToFlip(event.key, xform)) {
    updateStatus(*event.status);
    return ToolChanges::cell;
  }
  return ToolChanges::none;
}

namespace {

void applyTransform(Image &image) {
  QImage temp = makeCompatible(image.data);
  blitTransformedImage(temp, image);
  copyImage(image.data, temp);
}

}

void FlipTool::updateSourceImage() {
  assert(source);
  assert(!transform);
  applyTransform(source->image);
  source->image.xform.flipX = false;
  source->image.xform.flipY = false;
}

void FlipTool::updateStatus(std::string &status) {
  Transform &xform = getTransform(source, transform);
  status += "X: ";
  statusBool(status, xform.flipX);
  status += " Y: ";
  statusBool(status, xform.flipY);
}

bool RotateTool::attachCell(Cell *cell) {
  if ((source = dynamic_cast<SourceCell *>(cell))) {
    return true;
  } else if ((transform = dynamic_cast<TransformCell *>(cell))) {
    return true;
  }
  return false;
}

void RotateTool::detachCell() {
  if (source) {
    updateSourceImage();
  }
}

namespace {

quint8 arrowToRot(const Qt::Key key) {
  switch (key) {
    case key_rot_cw_a:
    case key_rot_cw_b: return 1;
    case key_rot_ccw_a:
    case key_rot_ccw_b: return 3;
    default: return 0;
  }
}

}

ToolChanges RotateTool::mouseMove(const ToolMouseEvent &event) {
  updateStatus(*event.status);
  return ToolChanges::none;
}

ToolChanges RotateTool::keyPress(const ToolKeyEvent &event) {
  assert(oneNotNull(source, transform));
  const quint8 rot = arrowToRot(event.key);
  if (rot) {
    quint8 &angle = getTransform(source, transform).angle;
    angle = (angle + rot) & 3;
    updateStatus(*event.status);
    return ToolChanges::cell;
  }
  return ToolChanges::none;
}

void RotateTool::updateSourceImage() {
  assert(source);
  assert(!transform);
  applyTransform(source->image);
  source->image.xform.angle = 0;
}

void RotateTool::updateStatus(std::string &status) {
  status += "ANGLE: ";
  status += std::to_string(getTransform(source, transform).angle * 90);
}
