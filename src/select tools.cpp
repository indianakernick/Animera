//
//  select tools.cpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "select tools.hpp"

#include "cell.hpp"
#include "connect.hpp"
#include "painting.hpp"
#include "composite.hpp"
#include "scope time.hpp"
#include "surface factory.hpp"
#include "graphics convert.hpp"
#include <Graphics/flood fill.hpp>

template <typename Derived>
SelectTool<Derived>::~SelectTool() {
  static_assert(std::is_base_of_v<SelectTool, Derived>);
}

template <typename Derived>
bool SelectTool<Derived>::resizeImages() {
  SCOPE_TIME("SelectTool::resizeImages");
  
  const QImage::Format format = qimageFormat(ctx->format);
  if (selection.format() != format || selection.size() != ctx->size) {
    selection = {ctx->size, format};
  }
  if (overlay.size() != ctx->size) {
    overlay = {ctx->size, qimageFormat(Format::rgba)};
    return true;
  }
  return false;
}

template <typename Derived>
void SelectTool<Derived>::copy(const QPoint pos) {
  SCOPE_TIME("SelectTool::copy");
  
  QImage overlayView = view(overlay, bounds);
  const QRect rect = bounds.intersected(ctx->cell->rect());
  if (rect.isEmpty()) {
    writeOverlay(ctx->palette, ctx->format, overlayView);
  } else {
    blitImage(
      selection,
      cview(ctx->cell->img, rect.translated(-ctx->cell->pos)),
      rect.topLeft()
    );
    writeOverlay(
      ctx->palette,
      ctx->format,
      overlayView,
      cview(selection, bounds)
    );
  }
  offset = bounds.topLeft() - pos;
}

template <typename Derived>
void SelectTool<Derived>::copyWithMask(
  const QPoint pos,
  const QImage &mask
) {
  SCOPE_TIME("SelectTool::copyWithMask");
  
  const QRect rect = bounds.intersected(ctx->cell->rect());
  if (!rect.isEmpty()) {
    blitMaskImage(
      selection,
      cview(mask, rect),
      cview(ctx->cell->img, rect.translated(-ctx->cell->pos)),
      rect.topLeft()
    );
  }
  QImage overlayView = view(overlay, bounds);
  writeOverlay(
    ctx->palette,
    ctx->format,
    overlayView,
    cview(selection, bounds),
    cview(mask, bounds)
  );
  offset = bounds.topLeft() - pos;
}

template <typename Derived>
void SelectTool<Derived>::paste(
  const QPoint pos,
  const ButtonType button
) {
  SCOPE_TIME("SelectTool::paste");
  
  const QRect rect = overlayRect(pos);
  if (button == ButtonType::secondary) {
    return ctx->changeOverlay(rect);
  }
  if (button == ButtonType::primary) {
    ctx->growCell(rect);
    const QPoint cellPos = ctx->cell->pos;
    blitImage(ctx->cell->img, cview(selection, bounds), rect.topLeft() - cellPos);
    ctx->shrinkCell(rect);
  } else if (button == ButtonType::erase) {
    if (ctx->colors.erase != 0) ctx->growCell(rect);
    const QPoint cellPos = ctx->cell->pos;
    drawFilledRect(ctx->cell->img, ctx->colors.erase, rect.translated(-cellPos));
    if (ctx->colors.erase == 0) ctx->shrinkCell(rect);
  }
  ctx->changeCell(rect);
  ctx->finishChange();
}

template <typename Derived>
void SelectTool<Derived>::pasteWithMask(
  const QPoint pos,
  const ButtonType button,
  const QImage &mask
) {
  SCOPE_TIME("SelectTool::pasteWithMask");
  
  const QRect rect = overlayRect(pos);
  if (button == ButtonType::secondary) {
    return ctx->changeOverlay(rect);
  }
  if (button == ButtonType::primary) {
    ctx->growCell(rect);
    const QPoint offsetPos = rect.topLeft() - ctx->cell->pos;
    blitMaskImage(ctx->cell->img, cview(mask, bounds), cview(selection, bounds), offsetPos);
    ctx->shrinkCell(rect);
  } else if (button == ButtonType::erase) {
    if (ctx->colors.erase != 0) ctx->growCell(rect);
    const QPoint offsetPos = rect.topLeft() - ctx->cell->pos;
    fillMaskImage(ctx->cell->img, cview(mask, bounds), ctx->colors.erase, offsetPos);
    if (ctx->colors.erase == 0) ctx->shrinkCell(rect);
  }
  ctx->changeCell(rect);
  ctx->finishChange();
}

template <typename Derived>
QRect SelectTool<Derived>::overlayRect(const QPoint pos) {
  return {pos + offset, bounds.size()};
}

template <typename Derived>
void SelectTool<Derived>::showOverlay(const QPoint pos) {
  SCOPE_TIME("SelectTool::showOverlay");
  
  blitImage(*ctx->overlay, cview(overlay, bounds), pos + offset);
}

template <typename Derived>
void SelectTool<Derived>::clearOverlay(const QPoint pos) {
  SCOPE_TIME("SelectTool::clearOverlay");
  
  clearImage(*ctx->overlay, overlayRect(pos));
}

template <typename Derived>
void SelectTool<Derived>::clearOverlay(const SelectMode currMode, const QPoint pos) {
  if (currMode == SelectMode::copy) {
    drawSquarePoint(*ctx->overlay, 0, pos);
    ctx->changeOverlay(pos);
  } else if (currMode == SelectMode::paste) {
    clearOverlay(pos);
    ctx->changeOverlay(overlayRect(pos));
  } else Q_UNREACHABLE();
}

template <typename Derived>
void SelectTool<Derived>::toggleMode() {
  if (mode == SelectMode::copy) {
    if (!bounds.isEmpty()) mode = SelectMode::paste;
  } else if (mode == SelectMode::paste) {
    mode = SelectMode::copy;
  } else Q_UNREACHABLE();
}

void RectangleSelectTool::attachCell() {
  SCOPE_TIME("RectangleSelectTool::attachCell");
  
  resizeImages();
}

void RectangleSelectTool::mouseLeave(const ToolLeaveEvent &event) {
  SCOPE_TIME("RectangleSelectTool::mouseLeave");
  
  ctx->clearStatus();
  clearOverlay(mode, event.lastPos);
}

void RectangleSelectTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("RectangleSelectTool::mouseDown");
  
  if (event.button == ButtonType::secondary) {
    const SelectMode prevMode = mode;
    toggleMode();
    if (prevMode != mode) {
      clearOverlay(prevMode, event.lastPos);
    }
  }
  
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      startPos = event.pos;
      ctx->lock();
      status.append("SELECTION: ");
      status.append({event.pos, QSize{1, 1}});
    } else {
      status.appendLabeled(event.pos);
    }
    drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
    ctx->changeOverlay(event.pos);
  } else if (mode == SelectMode::paste) {
    status.append("SELECTION: ");
    status.append(overlayRect(event.pos));
    showOverlay(event.pos);
    paste(event.pos, event.button);
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
}

void RectangleSelectTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("RectangleSelectTool::mouseMove");
  
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      drawStrokedRect(*ctx->overlay, 0, bounds);
      bounds = unite(startPos, event.pos);
      drawStrokedRect(*ctx->overlay, tool_overlay_color, bounds);
      ctx->changeOverlay(bounds.united(toRect(event.lastPos)));
      status.append("SELECTION: ");
      status.append(bounds);
    } else {
      drawSquarePoint(*ctx->overlay, 0, event.lastPos);
      drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
      ctx->changeOverlay(unite(event.lastPos, event.pos));
      status.appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    clearOverlay(event.lastPos);
    showOverlay(event.pos);
    ctx->changeOverlay(overlayRect(event.pos).united(overlayRect(event.lastPos)));
    status.append("SELECTION: ");
    status.append(overlayRect(event.pos));
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
}

void RectangleSelectTool::mouseUp(const ToolMouseEvent &event) {
  SCOPE_TIME("RectangleSelectTool::mouseUp");
  
  if (event.button != ButtonType::primary) return;
  if (mode == SelectMode::copy) {
    ctx->unlock();
    bounds = unite(startPos, event.pos);
    clearImage(selection, lastBounds);
    lastBounds = bounds;
    copy(event.pos);
    mode = SelectMode::paste;
  }
  showOverlay(event.pos);
  ctx->changeOverlay(overlayRect(event.pos));
  StatusMsg status;
  status.appendLabeled(mode);
  status.append("SELECTION: ");
  status.append(overlayRect(event.pos));
  ctx->showStatus(status);
}

void PolygonSelectTool::attachCell() {
  SCOPE_TIME("PolygonSelectTool::attachCell");
  
  if (resizeImages()) {
    mask = {ctx->size, mask_format};
  }
}

void PolygonSelectTool::mouseLeave(const ToolLeaveEvent &event) {
  SCOPE_TIME("PolygonSelectTool::mouseLeave");
  
  ctx->clearStatus();
  clearOverlay(mode, event.lastPos);
}

void PolygonSelectTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("PolygonSelectTool::mouseDown");
  
  if (event.button == ButtonType::secondary) {
    const SelectMode prevMode = mode;
    toggleMode();
    if (prevMode != mode) {
      clearOverlay(prevMode, event.lastPos);
    }
  }
  
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      initPoly(event.pos);
      ctx->lock();
      status.append("SELECTION: ");
      status.append({event.pos, QSize{1, 1}});
    } else {
      status.appendLabeled(event.pos);
    }
    drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
    ctx->changeOverlay(event.pos);
  } else if (mode == SelectMode::paste) {
    status.append("SELECTION: ");
    status.append({event.pos + offset, bounds.size()});
    showOverlay(event.pos);
    pasteWithMask(event.pos, event.button, mask);
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
}

void PolygonSelectTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("PolygonSelectTool::mouseMove");
  
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      clearImage(*ctx->overlay, bounds);
      pushPoly(event.pos);
      drawFilledPolygon(*ctx->overlay, tool_overlay_color, polygon);
      ctx->changeOverlay(bounds);
      status.append("SELECTION: ");
      status.append(bounds);
    } else {
      drawSquarePoint(*ctx->overlay, 0, event.lastPos);
      drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
      ctx->changeOverlay(unite(event.lastPos, event.pos));
      status.appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    clearOverlay(event.lastPos);
    showOverlay(event.pos);
    ctx->changeOverlay(overlayRect(event.pos).united(overlayRect(event.lastPos)));
    status.append("SELECTION: ");
    status.append(overlayRect(event.pos));
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
}

void PolygonSelectTool::mouseUp(const ToolMouseEvent &event) {
  SCOPE_TIME("PolygonSelectTool::mouseUp");
  
  if (event.button != ButtonType::primary) return;
  if (mode == SelectMode::copy) {
    ctx->unlock();
    pushPoly(event.pos);
    clearImage(mask, lastBounds);
    drawFilledPolygon(mask, mask_color_on, polygon);
    clearImage(selection, lastBounds);
    lastBounds = bounds;
    copyWithMask(event.pos, mask);
    mode = SelectMode::paste;
  }
  showOverlay(event.pos);
  ctx->changeOverlay(overlayRect(event.pos));
  StatusMsg status;
  status.appendLabeled(mode);
  status.append("SELECTION: ");
  status.append(overlayRect(event.pos));
  ctx->showStatus(status);
}

void PolygonSelectTool::initPoly(const QPoint point) {
  polygon.clear();
  polygon.push_back(point);
  bounds = toRect(point);
}

void PolygonSelectTool::pushPoly(const QPoint point) {
  assert(!polygon.empty());
  if (polygon.back() == point) return;
  polygon.push_back(point);
  bounds = bounds.united(toRect(point));
}

WandSelectTool::WandSelectTool() {
  animFrame = 0;
  animTimer.setInterval(wand_interval);
  CONNECT_LAMBDA(animTimer, timeout, [this]{ animate(); });
}

void WandSelectTool::attachCell() {
  SCOPE_TIME("WandSelectTool::attachCell");
  
  mode = SelectMode::copy;
  if (resizeImages()) {
    mask = {ctx->size, mask_format};
  }
  clearImage(mask, bounds);
  bounds = {};
  animTimer.start();
}

void WandSelectTool::detachCell() {
  SCOPE_TIME("WandSelectTool::detachCell");
  
  animTimer.stop();
  if (mode == SelectMode::copy) {
    clearImage(*ctx->overlay, bounds);
    ctx->changeOverlay(bounds);
  }
}

void WandSelectTool::mouseLeave(const ToolLeaveEvent &event) {
  SCOPE_TIME("WandSelectTool::mouseLeave");
  
  ctx->clearStatus();
  if (mode == SelectMode::paste) {
    const QRect rect = overlayRect(event.lastPos);
    clearImage(*ctx->overlay, rect);
    ctx->changeOverlay(rect);
  }
}

void WandSelectTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("WandSelectTool::mouseDown");
  
  if (event.button == ButtonType::secondary) {
    toggleMode(event);
  }

  StatusMsg status;
  status.appendLabeled(mode);

  if (mode == SelectMode::copy) {
    status.appendLabeled(event.pos);
    if (event.button == ButtonType::primary) {
      addToSelection(event);
    }
  } else if (mode == SelectMode::paste) {
    status.append("SELECTION: ");
    status.append({event.pos + offset, bounds.size()});
    clearOverlay(event.lastPos);
    showOverlay(event.pos);
    pasteWithMask(event.pos, event.button, mask);
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
}

void WandSelectTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("WandSelectTool::mouseMove");
  
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    status.appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    status.append("SELECTION: ");
    status.append(overlayRect(event.pos));
    clearOverlay(event.lastPos);
    showOverlay(event.pos);
    ctx->changeOverlay(overlayRect(event.lastPos).united(overlayRect(event.pos)));
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
}

QRect WandSelectTool::cellRect() const {
  return toRect(ctx->size).intersected(ctx->cell->rect());
}

void WandSelectTool::toggleMode(const ToolMouseEvent &event) {
  SCOPE_TIME("WandSelectTool::toggleMode");
  
  if (mode == SelectMode::paste) {
    mode = SelectMode::copy;
    const QRect rect = overlayRect(event.lastPos);
    clearImage(*ctx->overlay, rect);
    ctx->changeOverlay(rect);
    clearImage(overlay, bounds);
    clearImage(mask, bounds);
    bounds = {};
    animTimer.start();
  } else if (mode == SelectMode::copy) {
    if (bounds.isEmpty()) return;
    animTimer.stop();
    mode = SelectMode::paste;
    clearImage(*ctx->overlay, bounds);
    ctx->changeOverlay(bounds);
    clearImage(selection, bounds);
    copyWithMask(event.pos, mask);
    ctx->shrinkCell();
  } else Q_UNREACHABLE();
}

namespace {

template <typename Pixel>
class WandPolicy {
public:
  WandPolicy(gfx::Surface<PixelMask> mask, gfx::CSurface<Pixel> source)
    : mask{mask}, source{source} {}

  bool start(const gfx::Point pos) {
    startColor = source.ref(pos);
    maskCheckColor = mask.ref(pos);
    maskColor = ~maskCheckColor;
    return true;
  }
  
  bool removed() const {
    return maskColor == 0;
  }
  
  gfx::Size size() const {
    return source.size();
  }
  
  bool check(const gfx::Point pos) const {
    return source.ref(pos) == startColor &&
           mask.ref(pos) == maskCheckColor;
  }
  
  void set(const gfx::Point pos) const {
    mask.ref(pos) = maskColor;
  }

private:
  gfx::Surface<PixelMask> mask;
  gfx::CSurface<Pixel> source;
  Pixel startColor;
  PixelMask maskColor;
  PixelMask maskCheckColor;
};

}

void WandSelectTool::addToSelection(const ToolMouseEvent &event) {
  SCOPE_TIME("WandSelectTool::addToSelection");
  
  // TODO: do something similar to flood fill optimization
  QRect rect = toRect(ctx->size);
  if (sampleCell(*ctx->cell, event.pos) == 0) {
    ctx->growCell(rect);
  } else {
    rect = rect.intersected(ctx->cell->rect());
  }
  const gfx::Surface maskSurface = makeSurface<PixelMask>(mask).view(convert(rect));
  const gfx::Point cellPos = convert(event.pos - rect.topLeft());
  const gfx::Rect cellRect = convert(rect.translated(-ctx->cell->pos));
  bool removedFromSelection = false;

  switch (ctx->format) {
    case Format::rgba: {
      gfx::Surface surface = makeCSurface<PixelRgba>(ctx->cell->img).view(cellRect);
      WandPolicy policy{maskSurface, surface};
      bounds = bounds.united(convert(gfx::floodFill(policy, cellPos)).translated(rect.topLeft()));
      removedFromSelection = policy.removed();
      break;
    }
    case Format::index: {
      gfx::Surface surface = makeCSurface<PixelIndex>(ctx->cell->img).view(cellRect);
      WandPolicy policy{maskSurface, surface};
      bounds = bounds.united(convert(gfx::floodFill(policy, cellPos)).translated(rect.topLeft()));
      removedFromSelection = policy.removed();
      break;
    }
    case Format::gray: {
      gfx::Surface surface = makeCSurface<PixelGray>(ctx->cell->img).view(cellRect);
      WandPolicy policy{maskSurface, surface};
      bounds = bounds.united(convert(gfx::floodFill(policy, cellPos)).translated(rect.topLeft()));
      removedFromSelection = policy.removed();
      break;
    }
  }
  
  if (removedFromSelection) {
    clearImage(*ctx->overlay, rect);
  }
  paintOverlay();
}

QRgb WandSelectTool::getOverlayColor() const {
  static_assert(wand_frames % 2 == 0);
  constexpr int half_frames = wand_frames / 2;
  const int mirroredFrame = animFrame > half_frames ? wand_frames - animFrame : animFrame;
  const int gray = scale(mirroredFrame, half_frames, 255);
  return qRgba(gray, gray, gray, wand_alpha);
}

void WandSelectTool::paintOverlay() const {
  SCOPE_TIME("WandSelectTool::paintOverlay");
  
  if (mode != SelectMode::copy) return;
  if (bounds.isEmpty()) return;
  fillMaskImage(*ctx->overlay, cview(mask, bounds), getOverlayColor(), bounds.topLeft());
  ctx->changeOverlay(bounds);
}

void WandSelectTool::animate() {
  SCOPE_TIME("WandSelectTool::animate");
  
  paintOverlay();
  animFrame = (animFrame + 1) % wand_frames;
}
