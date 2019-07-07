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

ToolChanges BrushTool::mouseDown(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  symPoint(*event.overlay, tool_overlay_color, event.pos);
  symPoint(*event.status, event.pos);
  lastPos = event.pos;
  color = selectColor(event.colors, event.button);
  return drawnChanges(symPoint(cell->image, color, lastPos));
}

ToolChanges BrushTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  symPoint(*event.overlay, tool_overlay_color, event.pos);
  symPoint(*event.status, event.pos);
  if (event.button == ButtonType::none) return ToolChanges::overlay;
  const bool drawn = symLine(cell->image, color, {lastPos, event.pos});
  lastPos = event.pos;
  return drawnChanges(drawn);
}

ToolChanges BrushTool::mouseUp(const ToolMouseEvent &event) {
  symPoint(*event.status, event.pos);
  return drawnChanges(symLine(cell->image, color, {lastPos, event.pos}));
}

void BrushTool::setRadius(const int newRadius) {
  assert(brsh_min_radius <= newRadius && newRadius <= brsh_max_radius);
  radius = newRadius;
}

void BrushTool::setMode(const SymmetryMode newMode) {
  mode = newMode;
}

void BrushTool::symPoint(StatusMsg &status, const QPoint point) {
  const QSize size = cell->image.size();
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
  const QPoint refl = {img.width() - point.x() - 1, img.height() - point.y() - 1};
  bool drawn = drawRoundPoint(img, col, point, radius);
  if (mode & SymmetryMode::hori) {
    drawn |= drawRoundPoint(img, col, {refl.x(), point.y()}, radius);
  }
  if (mode & SymmetryMode::vert) {
    drawn |= drawRoundPoint(img, col, {point.x(), refl.y()}, radius);
  }
  if (mode & SymmetryMode::both) {
    drawn |= drawRoundPoint(img, col, refl, radius);
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
  bool drawn = drawLine(img, col, line, radius);
  if (mode & SymmetryMode::hori) {
    const QLine refl = {reflectX(size, line.p1()), reflectX(size, line.p2())};
    drawn |= drawLine(img, col, refl, radius);
  }
  if (mode & SymmetryMode::vert) {
    const QLine refl = {reflectY(size, line.p1()), reflectY(size, line.p2())};
    drawn |= drawLine(img, col, refl, radius);
  }
  if (mode & SymmetryMode::both) {
    const QLine refl = {reflectXY(size, line.p1()), reflectXY(size, line.p2())};
    drawn |= drawLine(img, col, refl, radius);
  }
  return drawn;
}

ToolChanges FloodFillTool::mouseDown(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
  event.status->appendLabeled(event.pos);
  const QRgb color = selectColor(event.colors, event.button);
  return drawnChanges(drawFloodFill(cell->image, color, event.pos));
}

ToolChanges FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
  event.status->appendLabeled(event.pos);
  return ToolChanges::overlay;
}

ToolChanges RectangleSelectTool::mouseDown(const ToolMouseEvent &event) {
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
      blitImage(cell->image, selection, event.pos + offset);
    } else if (event.button == ButtonType::erase) {
      const QRect rect{event.pos + offset, selection.size()};
      drawFilledRect(cell->image, event.colors.erase, rect);
    } else {
      return ToolChanges::overlay;
    }
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges RectangleSelectTool::mouseMove(const ToolMouseEvent &event) {
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
  if (event.button != ButtonType::primary) return ToolChanges::none;
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, tool_overlay_color, event.pos);
    const QRect rect = QRect{startPos, event.pos}.normalized();
    selection = blitImage(cell->image, rect);
    overlay = QImage{selection.size(), qimageFormat(Format::color)};
    writeOverlay(palette, format, overlay, selection);
    offset = rect.topLeft() - event.pos;
    mode = SelectMode::paste;
  }
  blitImage(*event.overlay, overlay, event.pos + offset);
  event.status->appendLabeled(mode);
  event.status->appendLabeled({event.pos + offset, overlay.size()});
  return ToolChanges::overlay;
}

ToolChanges PolygonSelectTool::mouseDown(const ToolMouseEvent &event) {
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
      blitMaskImage(cell->image, mask, selection, event.pos + offset);
    } else if (event.button == ButtonType::erase) {
      // @TODO should this be encapsulated in another file?
      visitSurface(cell->image, event.colors.erase, [this, &event](auto surface, auto color) {
        maskFillRegion(surface, makeCSurface<uint8_t>(mask), color, event.pos + offset);
      });
    } else {
      return ToolChanges::overlay;
    }
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges PolygonSelectTool::mouseMove(const ToolMouseEvent &event) {
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
  if (event.button != ButtonType::primary) return ToolChanges::none;
  clearImage(*event.overlay);
  QPolygon p;
  if (mode == SelectMode::copy) {
    polygon.push(event.pos);
    const QRect clippedBounds = polygon.bounds().intersected(cell->image.rect());
    mask = QImage{clippedBounds.size(), mask_format};
    clearImage(mask);
    drawFilledPolygon(mask, mask_color_on, polygon, -clippedBounds.topLeft());
    selection = blitMaskImage(cell->image, mask, clippedBounds.topLeft());
    overlay = QImage{selection.size(), qimageFormat(Format::color)};
    writeOverlay(palette, format, overlay, selection, mask);
    offset = clippedBounds.topLeft() - event.pos;
    mode = SelectMode::paste;
  }
  blitImage(*event.overlay, overlay, event.pos + offset);
  event.status->appendLabeled(mode);
  event.status->appendLabeled({event.pos + offset, overlay.size()});
  return ToolChanges::overlay;
}

void WandSelectTool::attachCell(Cell *newCell) {
  Tool::attachCell(newCell);
  mode = SelectMode::copy;
  if (!compatible(newCell->image, selection)) {
    selection = makeCompatible(cell->image);
    overlay = QImage{selection.size(), qimageFormat(Format::color)};
    mask = makeMask(selection.size());
  }
  clearImage(mask);
}

void WandSelectTool::detachCell() {
  // @TODO clear the overlay somehow
  // If we don't clear the overlay on mouseLeave then we have to clear it here
  Tool::detachCell();
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
      blitMaskImage(cell->image, mask, selection, event.pos + offset);
    } else if (event.button == ButtonType::erase) {
      visitSurface(cell->image, event.colors.erase, [this, &event](auto surface, auto color) {
        maskFillRegion(surface, makeCSurface<uint8_t>(mask), color, event.pos + offset);
      });
    } else {
      return ToolChanges::overlay;
    }
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges WandSelectTool::mouseMove(const ToolMouseEvent &event) {
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
    selection = blitMaskImage(cell->image, mask, {0, 0});
    writeOverlay(palette, format, overlay, selection, mask);
    offset = -event.pos;
    mode = SelectMode::paste;
  } else Q_UNREACHABLE();
}

namespace {

QRgb contrastColor(const QRgb color) {
  return qGray(color) < 128 ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
}

QRgb contrastGray(const int gray) {
  return gray < 128 ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
}

template <typename Pixel>
class WandManip {
public:
  WandManip(Surface<PixelColor> overlay, Surface<PixelMask> mask, CSurface<Pixel> source, const PixelColor constrastColor)
    : overlay{overlay}, mask{mask}, source{source}, constrastColor{constrastColor} {}

  bool start(const QPoint pos) {
    startColor = source.getPixel(pos);
    maskCheckColor = mask.getPixel(pos);
    maskColor = ~maskCheckColor;
    if (maskColor == mask_off) {
      overlayColor = qRgba(0, 0, 0, 0);
    } else {
      overlayColor = constrastColor;
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
  Surface<PixelColor> overlay;
  Surface<PixelMask> mask;
  CSurface<Pixel> source;
  Pixel startColor;
  PixelColor constrastColor;
  PixelColor overlayColor;
  PixelMask maskColor;
  PixelMask maskCheckColor;
};

}

void WandSelectTool::addToSelection(const ToolMouseEvent &event) {
  switch (format) {
    case Format::color: {
      Surface surface = makeCSurface<PixelColor>(cell->image);
      WandManip manip{
        makeSurface<PixelColor>(*event.overlay),
        makeSurface<PixelMask>(mask),
        surface,
        contrastColor(surface.getPixel(event.pos))
      };
      floodFill(manip, event.pos);
      break;
    }
    case Format::palette: {
      Surface surface = makeCSurface<PixelPalette>(cell->image);
      WandManip manip{
        makeSurface<PixelColor>(*event.overlay),
        makeSurface<PixelMask>(mask),
        surface,
        contrastColor(palette[surface.getPixel(event.pos)])
      };
      floodFill(manip, event.pos);
      break;
    }
    case Format::gray: {
      Surface surface = makeCSurface<PixelGray>(cell->image);
      WandManip manip{
        makeSurface<PixelColor>(*event.overlay),
        makeSurface<PixelMask>(mask),
        surface,
        contrastGray(surface.getPixel(event.pos))
      };
      floodFill(manip, event.pos);
      break;
    }
  }
}

template <typename Derived>
DragPaintTool<Derived>::~DragPaintTool() {
  static_assert(std::is_base_of_v<DragPaintTool, Derived>);
}

template <typename Derived>
void DragPaintTool<Derived>::attachCell(Cell *newCell) {
  Tool::attachCell(newCell);
  if (!compatible(cleanImage, cell->image)) {
    cleanImage = makeCompatible(cell->image);
  }
}

template <typename Derived>
void DragPaintTool<Derived>::detachCell() {
  Tool::detachCell();
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseDown(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  that()->updateStatus(*event.status, event.pos, event.pos);
  startPos = event.pos;
  copyImage(cleanImage, cell->image);
  color = selectColor(event.colors, event.button);
  return drawnChanges(that()->drawPoint(cell->image, startPos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseMove(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  if (event.button == ButtonType::none) {
    event.status->appendLabeled(event.pos);
    return ToolChanges::overlay;
  }
  that()->updateStatus(*event.status, startPos, event.pos);
  copyImage(cell->image, cleanImage);
  return drawnChanges(that()->drawDrag(cell->image, startPos, event.pos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseUp(const ToolMouseEvent &event) {
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  copyImage(cell->image, cleanImage);
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

void LineTool::setRadius(const int newRadius) {
  radius = newRadius;
}

bool LineTool::drawPoint(QImage &image, const QPoint pos) {
  return drawRoundPoint(image, getColor(), pos, radius);
}

bool LineTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawLine(image, getColor(), {start, end}, radius);
}

void LineTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawRoundPoint(overlay, tool_overlay_color, pos, radius);
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

bool StrokedCircleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos, shape);
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

bool StrokedCircleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawStrokedCircle(image, getColor(), start, calcRadius(start, end), thickness, shape);
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

bool FilledCircleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos, shape);
}

bool FilledCircleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  return drawFilledCircle(image, getColor(), start, calcRadius(start, end), shape);
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

bool StrokedRectangleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos);
}

bool StrokedRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  return drawStrokedRect(image, getColor(), rect, thickness);
}

void StrokedRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, tool_overlay_color, pos);
}

void StrokedRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.appendLabeled(QRect{start, end}.normalized());
}

FilledRectangleTool::~FilledRectangleTool() = default;

bool FilledRectangleTool::drawPoint(QImage &image, const QPoint pos) {
  return drawSquarePoint(image, getColor(), pos);
}

bool FilledRectangleTool::drawDrag(QImage &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  return drawFilledRect(image, getColor(), rect);
}

void FilledRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, tool_overlay_color, pos);
}

void FilledRectangleTool::updateStatus(StatusMsg &status, const QPoint start, const QPoint end) {
  status.appendLabeled(QRect{start, end}.normalized());
}

void TranslateTool::attachCell(Cell *newCell) {
  Tool::attachCell(newCell);
  if (!compatible(cleanImage, cell->image)) {
    cleanImage = makeCompatible(cell->image);
  }
  copyImage(cleanImage, cell->image);
  pos = {0, 0};
}

ToolChanges TranslateTool::mouseDown(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary) return ToolChanges::none;
  lastPos = event.pos;
  drag = true;
  return ToolChanges::none;
}

ToolChanges TranslateTool::mouseMove(const ToolMouseEvent &event) {
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
  QImage &src = cell->image;
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

void FlipTool::attachCell(Cell *newCell) {
  Tool::attachCell(newCell);
  flipX = flipY = false;
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
  updateStatus(*event.status);
  return ToolChanges::none;
}

ToolChanges FlipTool::keyPress(const ToolKeyEvent &event) {
  if (flipXChanged(event.key, flipX)) {
    QImage &src = cell->image;
    QImage flipped{src.size(), src.format()};
    if (src.depth() == 32) {
      flipHori(makeSurface<uint32_t>(flipped), makeCSurface<uint32_t>(src));
    } else if (src.depth() == 8) {
      flipHori(makeSurface<uint8_t>(flipped), makeCSurface<uint8_t>(src));
    } else Q_UNREACHABLE();
    src = flipped;
  } else if (flipYChanged(event.key, flipY)) {
    QImage &src = cell->image;
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

void RotateTool::attachCell(Cell *newCell) {
  Tool::attachCell(newCell);
  const QSize size = cell->image.size();
  square = size.width() == size.height();
  angle = 0;
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
  const quint8 rot = arrowToRot(event.key);
  if (square && rot) {
    angle = (angle + rot) & 3;
    QImage &src = cell->image;
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
    status.append("Only square sprites can be rotated");
  }
}

// @TODO This file is almost 1000 lines
