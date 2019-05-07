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

// @TODO this file is starting to get out of hand

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
  source = nullptr;
}

ToolChanges BrushTool::mouseDown(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  symPoint(*event.overlay, tool_overlay_color, event.pos);
  symPoint(*event.status, event.pos);
  lastPos = event.pos;
  color = selectColor(event.colors, event.button);
  return drawnChanges(symPoint(source->image.data, color, lastPos));
}

ToolChanges BrushTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  symPoint(*event.overlay, tool_overlay_color, event.pos);
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

void BrushTool::setWidth(const int newWidth) {
  assert(brsh_min_thick <= newWidth && newWidth <= brsh_max_thick);
  width = newWidth;
}

void BrushTool::setMode(const SymmetryMode newMode) {
  mode = newMode;
}

void BrushTool::symPoint(StatusMsg &status, const QPoint point) {
  const QSize size = source->image.data.size();
  const QPoint refl = {size.width() - point.x() - 1, size.height() - point.y() - 1};
  status.appendLabeled(point);
  if (mode & SymmetryMode::hori) {
    status.append(' ');
    status.append(QPoint{refl.x(), point.y()});
  }
  if (mode & SymmetryMode::vert) {
    status.append(' ');
    status.append(QPoint{point.x(), refl.y()});
  }
  if (mode & SymmetryMode::both) {
    status.append(' ');
    status.append(refl);
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
  drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
  event.status->appendLabeled(event.pos);
  const QRgb color = selectColor(event.colors, event.button);
  return drawnChanges(drawFloodFill(source->image.data, color, event.pos));
}

ToolChanges FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
  event.status->appendLabeled(event.pos);
  return ToolChanges::overlay;
}

bool RectangleSelectTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void RectangleSelectTool::detachCell() {
  assert(source);
  source = nullptr;
}

ToolChanges RectangleSelectTool::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  if (event.button == ButtonType::secondary && !overlay.isNull()) {
    mode = opposite(mode);
  }
  event.status->appendLabeled(mode);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
    event.status->appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    event.status->appendLabeled({event.pos + offset, overlay.size()});
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
  event.status->appendLabeled(mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      const QRect rect = QRect{startPos, event.pos}.normalized();
      drawStrokedRect(*event.overlay, tool_overlay_color, rect, 1);
      event.status->appendLabeled(rect);
    } else {
      drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
      event.status->appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    event.status->appendLabeled({event.pos + offset, overlay.size()});
  } else Q_UNREACHABLE();
  return ToolChanges::overlay;
}

ToolChanges RectangleSelectTool::mouseUp(const ToolMouseEvent &event) {
  assert(source);
  if (event.button != ButtonType::primary) return ToolChanges::none;
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
    const QRect rect = QRect{startPos, event.pos}.normalized();
    selection = blitImage(source->image.data, rect);
    overlay = selection;
    colorToOverlay(overlay);
    offset = rect.topLeft() - event.pos;
    mode = SelectMode::paste;
  }
  blitImage(*event.overlay, overlay, event.pos + offset);
  event.status->appendLabeled(mode);
  event.status->appendLabeled({event.pos + offset, overlay.size()});
  return ToolChanges::overlay;
}

bool PolygonSelectTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void PolygonSelectTool::detachCell() {
  assert(source);
  source = nullptr;
}

ToolChanges PolygonSelectTool::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  if (event.button == ButtonType::secondary && !overlay.isNull()) {
    mode = opposite(mode);
  }
  event.status->appendLabeled(mode);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
    event.status->appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    event.status->appendLabeled({event.pos + offset, overlay.size()});
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

ToolChanges PolygonSelectTool::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  event.status->appendLabeled(mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      polygon.push(event.pos);
      drawFilledPolygon(*event.overlay, tool_overlay_color, polygon, QPoint{0, 0});
      event.status->appendLabeled(polygon.bounds());
    } else {
      drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
      event.status->appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    event.status->appendLabeled({event.pos + offset, overlay.size()});
  } else Q_UNREACHABLE();
  return ToolChanges::overlay;
}

ToolChanges PolygonSelectTool::mouseUp(const ToolMouseEvent &event) {
  assert(source);
  if (event.button != ButtonType::primary) return ToolChanges::none;
  clearImage(*event.overlay);
  QPolygon p;
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
  event.status->appendLabeled(mode);
  event.status->appendLabeled({event.pos + offset, overlay.size()});
  return ToolChanges::overlay;
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
    event.status->appendLabeled(event.pos);
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
  drawSquarePoint(overlay, tool_overlay_color, pos);
}

void LineTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("START: ");
  status.append(start);
  status.append(" END: ");
  status.append(end);
}

StrokedCircleTool::~StrokedCircleTool() = default;

void StrokedCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
}

bool StrokedCircleTool::drawPoint(Image &, QPoint) {
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
  return drawStrokedCircle(image.data, getColor(), start, shape, calcRadius(start, end), thickness);
}

void StrokedCircleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawFilledRect(overlay, tool_overlay_color, centerRect(pos, shape));
}

void StrokedCircleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("CENTER: ");
  status.append(start);
  status.append(" RADIUS: ");
  status.append(calcRadius(start, end));
}

FilledCircleTool::~FilledCircleTool() = default;

void FilledCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
}

bool FilledCircleTool::drawPoint(Image &, QPoint) {
  return false;
}

bool FilledCircleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  return drawFilledCircle(image.data, getColor(), start, shape, calcRadius(start, end));
}

void FilledCircleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawFilledRect(overlay, tool_overlay_color, centerRect(pos, shape));
}

void FilledCircleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.append("CENTER: ");
  status.append(start);
  status.append(" RADIUS: ");
  status.append(calcRadius(start, end));
}

StrokedRectangleTool::~StrokedRectangleTool() = default;

bool StrokedRectangleTool::drawPoint(Image &image, const QPoint pos) {
  return drawSquarePoint(image.data, getColor(), pos);
}

bool StrokedRectangleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  return drawStrokedRect(image.data, getColor(), rect, thickness);
}

void StrokedRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, tool_overlay_color, pos);
}

void StrokedRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.appendLabeled(QRect{start, end}.normalized());
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
  drawSquarePoint(overlay, tool_overlay_color, pos);
}

void FilledRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.appendLabeled(QRect{start, end}.normalized());
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

void TranslateTool::updateStatus(StatusMsg &status) {
  if (source) {
    status.appendLabeled(pos);
  } else if (transform) {
    Transform &xform = transform->xform;
    status.appendLabeled({xform.posX, xform.posY});
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

void FlipTool::updateStatus(StatusMsg &status) {
  Transform &xform = getTransform(source, transform);
  status.append("X: ");
  status.append(xform.flipX);
  status.append(" Y: ");
  status.append(xform.flipY);
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

void RotateTool::updateStatus(StatusMsg &status) {
  status.append("ANGLE: ");
  status.append(getTransform(source, transform).angle * 90);
}
