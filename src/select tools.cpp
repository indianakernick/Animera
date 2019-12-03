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
  
  if (button == ButtonType::secondary) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  const QRect rect{pos + offset, bounds.size()};
  ctx->growCell(rect);
  const QPoint cellPos = ctx->cell->pos;
  const QPoint offsetPos = rect.topLeft() - cellPos;
  if (button == ButtonType::primary) {
    blitImage(ctx->cell->img, cview(selection, bounds), offsetPos);
  } else if (button == ButtonType::erase) {
    drawFilledRect(ctx->cell->img, ctx->colors.erase, rect.translated(-cellPos));
  }
  ctx->emitChanges(ToolChanges::cell_overlay);
  ctx->finishChange();
}

template <typename Derived>
void SelectTool<Derived>::pasteWithMask(
  const QPoint pos,
  const ButtonType button,
  const QImage &mask
) {
  SCOPE_TIME("SelectTool::pasteWithMask");
  
  if (button == ButtonType::secondary) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  const QRect rect = {pos + offset, bounds.size()};
  ctx->growCell(rect);
  const QPoint offsetPos = rect.topLeft() - ctx->cell->pos;
  if (button == ButtonType::primary) {
    blitMaskImage(ctx->cell->img, cview(mask, bounds), cview(selection, bounds), offsetPos);
  } else if (button == ButtonType::erase) {
    fillMaskImage(ctx->cell->img, cview(mask, bounds), ctx->colors.erase, offsetPos);
  }
  ctx->emitChanges(ToolChanges::cell_overlay);
  ctx->finishChange();
}

template <typename Derived>
void SelectTool<Derived>::showOverlay(const QPoint pos) {
  SCOPE_TIME("SelectTool::showOverlay");
  
  blitImage(*ctx->overlay, cview(overlay, bounds), pos + offset);
}

template <typename Derived>
void SelectTool<Derived>::toggleMode() {
  SCOPE_TIME("SelectTool::toggleMode");
  
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

void RectangleSelectTool::mouseLeave(const ToolLeaveEvent &) {
  SCOPE_TIME("RectangleSelectTool::mouseLeave");
  
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void RectangleSelectTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("RectangleSelectTool::mouseDown");
  
  clearImage(*ctx->overlay);
  if (event.button == ButtonType::secondary) {
    toggleMode();
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
    ctx->emitChanges(ToolChanges::overlay);
  } else if (mode == SelectMode::paste) {
    status.append("SELECTION: ");
    status.append({event.pos + offset, bounds.size()});
    showOverlay(event.pos);
    paste(event.pos, event.button);
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
}

void RectangleSelectTool::mouseMove(const ToolMouseEvent &event) {
  SCOPE_TIME("RectangleSelectTool::mouseMove");
  
  clearImage(*ctx->overlay);
  StatusMsg status;
  status.appendLabeled(mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      bounds = unite(startPos, event.pos);
      drawStrokedRect(*ctx->overlay, tool_overlay_color, bounds);
      status.append("SELECTION: ");
      status.append(bounds);
    } else {
      drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
      status.appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    showOverlay(event.pos);
    status.append("SELECTION: ");
    status.append({event.pos + offset, bounds.size()});
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void RectangleSelectTool::mouseUp(const ToolMouseEvent &event) {
  SCOPE_TIME("RectangleSelectTool::mouseUp");
  
  if (event.button != ButtonType::primary) return;
  if (mode == SelectMode::copy) {
    ctx->unlock();
    bounds = unite(startPos, event.pos);
    clearImage(selection);
    copy(event.pos);
    mode = SelectMode::paste;
  }
  clearImage(*ctx->overlay);
  showOverlay(event.pos);
  StatusMsg status;
  status.appendLabeled(mode);
  status.append("SELECTION: ");
  status.append({event.pos + offset, bounds.size()});
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void PolygonSelectTool::attachCell() {
  SCOPE_TIME("PolygonSelectTool::attachCell");
  
  if (resizeImages()) {
    mask = {ctx->size, mask_format};
  }
}

void PolygonSelectTool::mouseLeave(const ToolLeaveEvent &) {
  SCOPE_TIME("PolygonSelectTool::mouseLeave");
  
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void PolygonSelectTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("PolygonSelectTool::mouseDown");
  
  clearImage(*ctx->overlay);
  if (event.button == ButtonType::secondary) {
    toggleMode();
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
    ctx->emitChanges(ToolChanges::overlay);
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
  
  clearImage(*ctx->overlay);
  StatusMsg status;
  status.appendLabeled(mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      pushPoly(event.pos);
      drawFilledPolygon(*ctx->overlay, tool_overlay_color, polygon);
      status.append("SELECTION: ");
      status.append(bounds);
    } else {
      drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
      status.appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    showOverlay(event.pos);
    status.append("SELECTION: ");
    status.append({event.pos + offset, bounds.size()});
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void PolygonSelectTool::mouseUp(const ToolMouseEvent &event) {
  SCOPE_TIME("PolygonSelectTool::mouseUp");
  
  if (event.button != ButtonType::primary) return;
  if (mode == SelectMode::copy) {
    ctx->unlock();
    pushPoly(event.pos);
    clearImage(mask);
    drawFilledPolygon(mask, mask_color_on, polygon);
    clearImage(selection);
    copyWithMask(event.pos, mask);
    mode = SelectMode::paste;
  }
  clearImage(*ctx->overlay);
  showOverlay(event.pos);
  StatusMsg status;
  status.appendLabeled(mode);
  status.append("SELECTION: ");
  status.append({event.pos + offset, bounds.size()});
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
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
  
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  mode = SelectMode::copy;
  if (resizeImages()) {
    mask = {ctx->size, mask_format};
  }
  clearImage(mask);
  animTimer.start();
}

void WandSelectTool::detachCell() {
  SCOPE_TIME("WandSelectTool::detachCell");
  
  animTimer.stop();
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
}

void WandSelectTool::mouseLeave(const ToolLeaveEvent &) {
  SCOPE_TIME("WandSelectTool::mouseLeave");
  
  if (mode == SelectMode::paste) {
    clearImage(*ctx->overlay);
    ctx->emitChanges(ToolChanges::overlay);
  }
  ctx->clearStatus();
}

void WandSelectTool::mouseDown(const ToolMouseEvent &event) {
  SCOPE_TIME("WandSelectTool::mouseDown");
  
  if (event.button == ButtonType::secondary) {
    toggleMode(event);
    ctx->emitChanges(ToolChanges::overlay);
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
    clearImage(*ctx->overlay);
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
    status.append({event.pos + offset, bounds.size()});
    clearImage(*ctx->overlay);
    showOverlay(event.pos);
    ctx->emitChanges(ToolChanges::overlay);
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
}

void WandSelectTool::toggleMode(const ToolMouseEvent &event) {
  SCOPE_TIME("WandSelectTool::toggleMode");
  
  if (mode == SelectMode::paste) {
    mode = SelectMode::copy;
    clearImage(*ctx->overlay);
    clearImage(overlay);
    clearImage(mask);
    bounds = {};
    animTimer.start();
  } else if (mode == SelectMode::copy) {
    animTimer.stop();
    if (bounds.isEmpty()) return;
    mode = SelectMode::paste;
    clearImage(selection);
    copyWithMask(event.pos, mask);
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
    // TODO: don't clear the whole overlay
    // just clear rect
    clearImage(*ctx->overlay);
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
  
  const QRect rect = toRect(ctx->size).intersected(ctx->cell->rect());
  if (rect.isEmpty()) return;
  fillMaskImage(*ctx->overlay, cview(mask, rect), getOverlayColor(), rect.topLeft());
  ctx->emitChanges(ToolChanges::overlay);
}

void WandSelectTool::animate() {
  SCOPE_TIME("WandSelectTool::animate");
  
  paintOverlay();
  animFrame = (animFrame + 1) % wand_frames;
}
