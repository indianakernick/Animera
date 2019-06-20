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
#include "masking.hpp"
#include "painting.hpp"
#include "transform.hpp"
#include "composite.hpp"
#include "flood fill.hpp"
#include "surface factory.hpp"

// @TODO this file is starting to get out of hand!

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

bool BrushTool::attachCell(Cell *newCell) {
  assert(newCell);
  cell = newCell;
  return true;
}

void BrushTool::detachCell() {
  assert(cell);
  cell = nullptr;
}

ToolChanges BrushTool::mouseDown(const ToolMouseEvent &event) {
  assert(cell);
  clearImage(*event.overlay);
  symPoint(*event.overlay, tool_overlay_color, event.pos);
  symPoint(*event.status, event.pos);
  lastPos = event.pos;
  color = selectColor(event.colors, event.button);
  return drawnChanges(symPoint(cell->image.data, color, lastPos));
}

ToolChanges BrushTool::mouseMove(const ToolMouseEvent &event) {
  assert(cell);
  clearImage(*event.overlay);
  symPoint(*event.overlay, tool_overlay_color, event.pos);
  symPoint(*event.status, event.pos);
  if (event.button == ButtonType::none) return ToolChanges::overlay;
  Image &img = cell->image;
  const bool drawn = symLine(img.data, color, {lastPos, event.pos});
  lastPos = event.pos;
  return drawnChanges(drawn);
}

ToolChanges BrushTool::mouseUp(const ToolMouseEvent &event) {
  assert(cell);
  symPoint(*event.status, event.pos);
  return drawnChanges(symLine(cell->image.data, color, {lastPos, event.pos}));
}

void BrushTool::setWidth(const int newWidth) {
  assert(brsh_min_thick <= newWidth && newWidth <= brsh_max_thick);
  width = newWidth;
}

void BrushTool::setMode(const SymmetryMode newMode) {
  mode = newMode;
}

void BrushTool::symPoint(StatusMsg &status, const QPoint point) {
  const QSize size = cell->image.data.size();
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
  bool drawn = drawLine(img, col, line, width);
  if (mode & SymmetryMode::hori) {
    const QLine refl = {reflectX(size, line.p1()), reflectX(size, line.p2())};
    drawn |= drawLine(img, col, refl, width);
  }
  if (mode & SymmetryMode::vert) {
    const QLine refl = {reflectY(size, line.p1()), reflectY(size, line.p2())};
    drawn |= drawLine(img, col, refl, width);
  }
  if (mode & SymmetryMode::both) {
    const QLine refl = {reflectXY(size, line.p1()), reflectXY(size, line.p2())};
    drawn |= drawLine(img, col, refl, width);
  }
  return drawn;
}

bool FloodFillTool::attachCell(Cell *newCell) {
  assert(newCell);
  cell = newCell;
  return true;
}

void FloodFillTool::detachCell() {
  assert(cell);
  cell = nullptr;
}

ToolChanges FloodFillTool::mouseDown(const ToolMouseEvent &event) {
  assert(cell);
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
  event.status->appendLabeled(event.pos);
  const QRgb color = selectColor(event.colors, event.button);
  return drawnChanges(drawFloodFill(cell->image.data, color, event.pos));
}

ToolChanges FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  assert(cell);
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
  event.status->appendLabeled(event.pos);
  return ToolChanges::overlay;
}

bool RectangleSelectTool::attachCell(Cell *newCell) {
  assert(newCell);
  cell = newCell;
  return true;
}

void RectangleSelectTool::detachCell() {
  assert(cell);
  cell = nullptr;
}

ToolChanges RectangleSelectTool::mouseDown(const ToolMouseEvent &event) {
  assert(cell);
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
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) startPos = event.pos;
    return ToolChanges::overlay;
  } else if (mode == SelectMode::paste) {
    if (event.button == ButtonType::primary) {
      blitImage(cell->image.data, selection, event.pos + offset);
    } else if (event.button == ButtonType::erase) {
      const QRect rect{event.pos + offset, selection.size()};
      drawFilledRect(cell->image.data, event.colors.erase, rect);
    } else {
      return ToolChanges::overlay;
    }
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges RectangleSelectTool::mouseMove(const ToolMouseEvent &event) {
  assert(cell);
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
  assert(cell);
  if (event.button != ButtonType::primary) return ToolChanges::none;
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
    const QRect rect = QRect{startPos, event.pos}.normalized();
    selection = blitImage(cell->image.data, rect);
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

bool PolygonSelectTool::attachCell(Cell *newCell) {
  assert(newCell);
  cell = newCell;
  return true;
}

void PolygonSelectTool::detachCell() {
  assert(cell);
  cell = nullptr;
}

ToolChanges PolygonSelectTool::mouseDown(const ToolMouseEvent &event) {
  assert(cell);
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
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) polygon.init(event.pos);
    return ToolChanges::overlay;
  } else if (mode == SelectMode::paste) {
    if (event.button == ButtonType::primary) {
      blitMaskImage(cell->image.data, mask, selection, event.pos + offset);
    } else if (event.button == ButtonType::erase) {
      // @TODO should this be encapsulated in another file?
      makeSurface(cell->image.data, event.colors.erase, [this, &event](auto surface, auto color) {
        maskFillRegion(surface, makeCSurface<uint8_t>(mask), color, event.pos + offset);
      });
    } else {
      return ToolChanges::overlay;
    }
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges PolygonSelectTool::mouseMove(const ToolMouseEvent &event) {
  assert(cell);
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
  assert(cell);
  if (event.button != ButtonType::primary) return ToolChanges::none;
  clearImage(*event.overlay);
  QPolygon p;
  if (mode == SelectMode::copy) {
    polygon.push(event.pos);
    const QRect clippedBounds = polygon.bounds().intersected(cell->image.data.rect());
    mask = QImage{clippedBounds.size(), mask_format};
    clearImage(mask);
    drawFilledPolygon(mask, mask_color_on, polygon, -clippedBounds.topLeft());
    selection = blitMaskImage(cell->image.data, mask, clippedBounds.topLeft());
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

// @TODO WAND SELECT BUG!!!
// Select something
// change to PASTE mode
// switch to another tool
// switch back
// overlay is colored

bool WandSelectTool::attachCell(Cell *newCell) {
  assert(newCell);
  cell = newCell;
  selection = makeCompatible(cell->image.data);
  overlay = makeCompatible(selection);
  mask = makeMask(selection.size());
  clearImage(mask);
  return true;
}

void WandSelectTool::detachCell() {
  // @TODO clear the overlay somehow
  // If we don't clear the overlay on mouseLeave then we have to clear it here
  assert(cell);
  cell = nullptr;
}

ToolChanges WandSelectTool::mouseLeave(const ToolLeaveEvent &event) {
  // @TODO Maybe we could cache event.overlay?
  if (mode == SelectMode::copy) {
    return ToolChanges::none;
  } else if (mode == SelectMode::paste) {
    clearImage(*event.overlay);
    return ToolChanges::overlay;
  } Q_UNREACHABLE();
}

ToolChanges WandSelectTool::mouseDown(const ToolMouseEvent &event) {
  assert(cell);
  if (event.button == ButtonType::secondary) {
    toggleMode(event);
  }
  event.status->appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    event.status->appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    event.status->appendLabeled({event.pos + offset, selection.size()});
    clearImage(*event.overlay);
    blitImage(*event.overlay, overlay, event.pos + offset);
  } else Q_UNREACHABLE();

  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      addToSelection(event);
    }
    return ToolChanges::overlay;
  } else if (mode == SelectMode::paste) {
    // @TODO this is very similar to PolygonSelectTool
    if (event.button == ButtonType::primary) {
      blitMaskImage(cell->image.data, mask, selection, event.pos + offset);
    } else if (event.button == ButtonType::erase) {
      makeSurface(cell->image.data, event.colors.erase, [this, &event](auto surface, auto color) {
        maskFillRegion(surface, makeCSurface<uint8_t>(mask), color, event.pos + offset);
      });
    } else {
      return ToolChanges::overlay;
    }
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges WandSelectTool::mouseMove(const ToolMouseEvent &event) {
  assert(cell);
  event.status->appendLabeled(mode);
  if (mode == SelectMode::copy) {
    event.status->appendLabeled(event.pos);
    return ToolChanges::none;
  } else if (mode == SelectMode::paste) {
    event.status->appendLabeled({event.pos + offset, selection.size()});
    clearImage(*event.overlay);
    blitImage(*event.overlay, overlay, event.pos + offset);
    return ToolChanges::overlay;
  } else Q_UNREACHABLE();
}

ToolChanges WandSelectTool::mouseUp(const ToolMouseEvent &) {
  assert(cell);
  return ToolChanges::none;
}

void WandSelectTool::toggleMode(const ToolMouseEvent &event) {
  mode = opposite(mode);
  if (mode == SelectMode::copy) {
    clearImage(*event.overlay);
    clearImage(overlay);
    clearImage(selection);
    clearImage(mask);
  } else if (mode == SelectMode::paste) {
    selection = blitMaskImage(cell->image.data, mask, {0, 0});
    copyImage(overlay, selection);
    colorToOverlay(overlay, mask);
    offset = -event.pos;
    mode = SelectMode::paste;
  } else Q_UNREACHABLE();
}

namespace {

// @TODO support palette images

class WandManip {
public:
  WandManip(Surface<QRgb> overlay, Surface<uint8_t> mask, CSurface<QRgb> source)
    : overlay{overlay}, mask{mask}, source{source} {}

  bool start(const QPoint pos) {
    startColor = source.getPixel(pos);
    maskCheckColor = mask.getPixel(pos);
    maskColor = ~maskCheckColor;
    if (maskColor == mask_off) {
      overlayColor = qRgba(0, 0, 0, 0);
    } else if (qGray(startColor) < 128) {
      overlayColor = qRgb(255, 255, 255);
    } else {
      overlayColor = qRgb(0, 0, 0);
    }
    return true;
  }
  
  QSize size() const {
    return source.size();
  }
  
  bool shouldSet(const QPoint pos) const {
    return source.getPixel(pos) == startColor &&
           mask.getPixel(pos) == maskCheckColor;
  }
  
  void set(const QPoint pos) const {
    overlay.setPixel(overlayColor, pos);
    mask.setPixel(maskColor, pos);
  }

private:
  Surface<QRgb> overlay;
  Surface<uint8_t> mask;
  CSurface<QRgb> source;
  QRgb startColor;
  QRgb overlayColor;
  uint8_t maskColor;
  uint8_t maskCheckColor;
};

}

void WandSelectTool::addToSelection(const ToolMouseEvent &event) {
  WandManip manip{
    makeSurface<QRgb>(*event.overlay),
    makeSurface<uint8_t>(mask),
    makeCSurface<QRgb>(cell->image.data)
  };
  floodFill(manip, event.pos);
}

template <typename Derived>
DragPaintTool<Derived>::~DragPaintTool() {
  static_assert(std::is_base_of_v<DragPaintTool, Derived>);
}

template <typename Derived>
bool DragPaintTool<Derived>::attachCell(Cell *newCell) {
  assert(newCell);
  cell = newCell;
  if (!compatible(cleanImage, cell->image.data)) {
    cleanImage = makeCompatible(cell->image.data);
  }
  return true;
}

template <typename Derived>
void DragPaintTool<Derived>::detachCell() {
  assert(cell);
  cell = nullptr;
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseDown(const ToolMouseEvent &event) {
  assert(cell);
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  that()->updateStatus(*event.status, event.pos, event.pos);
  startPos = event.pos;
  copyImage(cleanImage, cell->image.data);
  color = selectColor(event.colors, event.button);
  return drawnChanges(that()->drawPoint(cell->image, startPos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseMove(const ToolMouseEvent &event) {
  assert(cell);
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  if (event.button == ButtonType::none) {
    event.status->appendLabeled(event.pos);
    return ToolChanges::overlay;
  }
  that()->updateStatus(*event.status, startPos, event.pos);
  copyImage(cell->image.data, cleanImage);
  return drawnChanges(that()->drawDrag(cell->image, startPos, event.pos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseUp(const ToolMouseEvent &event) {
  assert(cell);
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  copyImage(cell->image.data, cleanImage);
  const bool drawn = that()->drawDrag(cell->image, startPos, event.pos);
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

void LineTool::setThick(const int newThick) {
  thickness = newThick;
}

bool LineTool::drawPoint(Image &image, const QPoint pos) {
  return drawRoundPoint(image.data, getColor(), pos, thickness);
}

bool LineTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  return drawLine(image.data, getColor(), {start, end}, thickness);
}

void LineTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawRoundPoint(overlay, tool_overlay_color, pos, thickness);
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

void StrokedCircleTool::setThick(const int newThick) {
  thickness = newThick;
}

bool StrokedCircleTool::drawPoint(Image &image, const QPoint pos) {
  return drawSquarePoint(image.data, getColor(), pos, shape);
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
  return drawStrokedCircle(image.data, getColor(), start, calcRadius(start, end), thickness, shape);
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

bool FilledCircleTool::drawPoint(Image &image, const QPoint pos) {
  return drawSquarePoint(image.data, getColor(), pos, shape);
}

bool FilledCircleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  return drawFilledCircle(image.data, getColor(), start, calcRadius(start, end), shape);
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

void StrokedRectangleTool::setThick(const int newThick) {
  thickness = newThick;
}

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

bool TranslateTool::attachCell(Cell *newCell) {
  assert(newCell);
  cell = newCell;
  if (!compatible(cleanImage, cell->image.data)) {
    cleanImage = makeCompatible(cell->image.data);
  }
  copyImage(cleanImage, cell->image.data);
  pos = {0, 0};
  return true;
}

void TranslateTool::detachCell() {
  assert(cell);
  cell = nullptr;
}

ToolChanges TranslateTool::mouseDown(const ToolMouseEvent &event) {
  assert(cell);
  if (event.button != ButtonType::primary) return ToolChanges::none;
  lastPos = event.pos;
  drag = true;
  return ToolChanges::none;
}

ToolChanges TranslateTool::mouseMove(const ToolMouseEvent &event) {
  assert(cell);
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
  assert(cell);
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
  assert(cell);
  QPoint move = arrowToDir(event.key);
  if (move == QPoint{0, 0}) return ToolChanges::none;
  translate(move, event.colors.erase);
  updateStatus(*event.status);
  return ToolChanges::cell;
}

void TranslateTool::translate(const QPoint move, const QRgb eraseColor) {
  pos += move;
  updateSourceImage(eraseColor);
}

void TranslateTool::updateSourceImage(const QRgb eraseColor) {
  QImage &src = cell->image.data;
  clearImage(src, eraseColor);
  if (src.depth() == 32) {
    copyRegion(makeSurface<uint32_t>(src), makeCSurface<uint32_t>(cleanImage), pos);
  } else if (src.depth() == 8) {
    copyRegion(makeSurface<uint8_t>(src), makeCSurface<uint8_t>(cleanImage), pos);
  } else Q_UNREACHABLE();
}

void TranslateTool::updateStatus(StatusMsg &status) {
  status.appendLabeled(pos);
}

bool FlipTool::attachCell(Cell *newCell) {
  assert(newCell);
  cell = newCell;
  flipX = flipY = false;
  return true;
}

void FlipTool::detachCell() {
  assert(cell);
  cell = nullptr;
}

namespace {

bool flipXChanged(const Qt::Key key, bool &flipX) {
  switch (key) {
    case key_flp_on_x: return !std::exchange(flipX, true);
    case key_flp_off_x: return std::exchange(flipX, false);
    default: return false;
  }
}

bool flipYChanged(const Qt::Key key, bool &flipY) {
  switch (key) {
    case key_flp_on_y: return !std::exchange(flipY, true);
    case key_flp_off_y: return std::exchange(flipY, false);
    default: return false;
  }
}

}

ToolChanges FlipTool::mouseMove(const ToolMouseEvent &event) {
  assert(cell);
  updateStatus(*event.status);
  return ToolChanges::none;
}

ToolChanges FlipTool::keyPress(const ToolKeyEvent &event) {
  assert(cell);
  if (flipXChanged(event.key, flipX)) {
    QImage &src = cell->image.data;
    QImage flipped{src.size(), src.format()};
    if (src.depth() == 32) {
      flipHori(makeSurface<uint32_t>(flipped), makeCSurface<uint32_t>(src));
    } else if (src.depth() == 8) {
      flipHori(makeSurface<uint8_t>(flipped), makeCSurface<uint8_t>(src));
    } else Q_UNREACHABLE();
    src = flipped;
  } else if (flipYChanged(event.key, flipY)) {
    QImage &src = cell->image.data;
    QImage flipped{src.size(), src.format()};
    if (src.depth() == 32) {
      flipVert(makeSurface<uint32_t>(flipped), makeCSurface<uint32_t>(src));
    } else if (src.depth() == 8) {
      flipVert(makeSurface<uint8_t>(flipped), makeCSurface<uint8_t>(src));
    } else Q_UNREACHABLE();
    src = flipped;
  } else {
    return ToolChanges::none;
  }
  updateStatus(*event.status);
  return ToolChanges::cell;
}

void FlipTool::updateStatus(StatusMsg &status) {
  status.append("X: ");
  status.append(flipX);
  status.append(" Y: ");
  status.append(flipY);
}

bool RotateTool::attachCell(Cell *newCell) {
  assert(newCell);
  cell = newCell;
  const QSize size = cell->image.data.size();
  square = size.width() == size.height();
  angle = 0;
  return true;
}

void RotateTool::detachCell() {
  assert(cell);
  cell = nullptr;
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
  assert(cell);
  updateStatus(*event.status);
  return ToolChanges::none;
}

ToolChanges RotateTool::keyPress(const ToolKeyEvent &event) {
  assert(cell);
  const quint8 rot = arrowToRot(event.key);
  if (square && rot) {
    angle = (angle + rot) & 3;
    QImage &src = cell->image.data;
    QImage rotated{src.size(), src.format()};
    if (src.depth() == 32) {
      rotate(makeSurface<uint32_t>(rotated), makeCSurface<uint32_t>(src), rot);
    } else if (src.depth() == 8) {
      rotate(makeSurface<uint8_t>(rotated), makeCSurface<uint8_t>(src), rot);
    } else Q_UNREACHABLE();
    src = rotated;
    updateStatus(*event.status);
    return ToolChanges::cell;
  }
  return ToolChanges::none;
}

void RotateTool::updateStatus(StatusMsg &status) {
  if (square) {
    status.append("ANGLE: ");
    status.append(angle * 90);
  } else {
    // I think this is a sensible limitation
    // Sprites for games are square 99% time
    status.append("ONLY SQUARE SPRITES CAN BE ROTATED");
  }
}

// @TODO This file is almost 1000 lines
