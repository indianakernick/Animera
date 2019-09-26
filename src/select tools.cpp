//
//  select tools.cpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "select tools.hpp"

#include "cell.hpp"
#include "painting.hpp"
#include "composite.hpp"
#include "surface factory.hpp"
#include "graphics convert.hpp"
#include <Graphics/flood fill.hpp>

template <typename Derived>
SelectTool<Derived>::~SelectTool() {
  static_assert(std::is_base_of_v<SelectTool, Derived>);
}

template <typename Derived>
bool SelectTool<Derived>::resizeImages() {
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
  QImage overlayView = view(overlay, bounds);
  if (ctx->cell->isNull()) {
    writeOverlay(ctx->palette, ctx->format, overlayView);
  } else {
    const QRect rect = bounds.intersected(ctx->cell->rect());
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
  if (*ctx->cell) {
    const QRect rect = bounds.intersected(ctx->cell->rect());
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
  if (button == ButtonType::secondary) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  const QRect rect{pos + offset, bounds.size()};
  ctx->requireCell(rect);
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
  if (button == ButtonType::secondary) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  const QRect rect = {pos + offset, bounds.size()};
  ctx->requireCell(rect);
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
  blitImage(*ctx->overlay, cview(overlay, bounds), pos + offset);
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
  resizeImages();
}

void RectangleSelectTool::detachCell() {
  ctx->clearStatus();
}

void RectangleSelectTool::mouseLeave(const ToolLeaveEvent &) {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void RectangleSelectTool::mouseDown(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  if (event.button == ButtonType::secondary) {
    toggleMode();
  }
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
    status.appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    showOverlay(event.pos);
    status.appendLabeled({event.pos + offset, bounds.size()});
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) startPos = event.pos;
    ctx->emitChanges(ToolChanges::overlay);
  } else if (mode == SelectMode::paste) {
    paste(event.pos, event.button);
  } else Q_UNREACHABLE();
}

void RectangleSelectTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  StatusMsg status;
  status.appendLabeled(mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      bounds = unite(startPos, event.pos);
      drawStrokedRect(*ctx->overlay, tool_overlay_color, bounds);
      status.appendLabeled(bounds);
    } else {
      drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
      status.appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    showOverlay(event.pos);
    status.appendLabeled({event.pos + offset, bounds.size()});
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void RectangleSelectTool::mouseUp(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary) return;
  if (mode == SelectMode::copy) {
    bounds = unite(startPos, event.pos);
    clearImage(selection);
    copy(event.pos);
    mode = SelectMode::paste;
  }
  clearImage(*ctx->overlay);
  showOverlay(event.pos);
  StatusMsg status;
  status.appendLabeled(mode);
  status.appendLabeled({event.pos + offset, bounds.size()});
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void PolygonSelectTool::attachCell() {
  if (resizeImages()) {
    mask = {ctx->size, mask_format};
  }
}

void PolygonSelectTool::detachCell() {
  ctx->clearStatus();
}

void PolygonSelectTool::mouseLeave(const ToolLeaveEvent &) {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void PolygonSelectTool::mouseDown(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  if (event.button == ButtonType::secondary) {
    toggleMode();
  }
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
    status.appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    showOverlay(event.pos);
    status.appendLabeled({event.pos + offset, bounds.size()});
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) initPoly(event.pos);
    ctx->emitChanges(ToolChanges::overlay);
  } else if (mode == SelectMode::paste) {
    pasteWithMask(event.pos, event.button, mask);
  } else Q_UNREACHABLE();
}

void PolygonSelectTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  StatusMsg status;
  status.appendLabeled(mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      pushPoly(event.pos);
      drawFilledPolygon(*ctx->overlay, tool_overlay_color, polygon);
      status.appendLabeled(bounds);
    } else {
      drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
      status.appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    showOverlay(event.pos);
    status.appendLabeled({event.pos + offset, bounds.size()});
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void PolygonSelectTool::mouseUp(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary) return;
  if (mode == SelectMode::copy) {
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
  status.appendLabeled({event.pos + offset, bounds.size()});
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

void WandSelectTool::attachCell() {
  mode = SelectMode::copy;
  if (resizeImages()) {
    mask = {ctx->size, mask_format};
  }
  clearImage(mask);
}

void WandSelectTool::detachCell() {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void WandSelectTool::mouseLeave(const ToolLeaveEvent &) {
  if (mode == SelectMode::paste) {
    clearImage(*ctx->overlay);
    ctx->emitChanges(ToolChanges::overlay);
  }
  ctx->clearStatus();
}

void WandSelectTool::mouseDown(const ToolMouseEvent &event) {
  if (event.button == ButtonType::secondary) {
    toggleMode(event);
  }
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    status.appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    status.appendLabeled({event.pos + offset, bounds.size()});
    clearImage(*ctx->overlay);
    showOverlay(event.pos);
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);

  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      addToSelection(event);
    }
    ctx->emitChanges(ToolChanges::overlay);
  } else if (mode == SelectMode::paste) {
    pasteWithMask(event.pos, event.button, mask);
  } else Q_UNREACHABLE();
}

void WandSelectTool::mouseMove(const ToolMouseEvent &event) {
  StatusMsg status;
  status.appendLabeled(mode);
  if (mode == SelectMode::copy) {
    status.appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    status.appendLabeled(event.pos + offset);
    clearImage(*ctx->overlay);
    showOverlay(event.pos);
    ctx->emitChanges(ToolChanges::overlay);
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
}

void WandSelectTool::mouseUp(const ToolMouseEvent &) {}

void WandSelectTool::toggleMode(const ToolMouseEvent &event) {
  if (mode == SelectMode::paste) {
    mode = SelectMode::copy;
    clearImage(*ctx->overlay);
    clearImage(overlay);
    clearImage(mask);
    bounds = {};
  } else if (mode == SelectMode::copy) {
    if (bounds.isEmpty()) return;
    mode = SelectMode::paste;
    clearImage(selection);
    copyWithMask(event.pos, mask);
  } else Q_UNREACHABLE();
}

namespace {

QRgb contrastColor(const QRgb color) {
  // TODO: contrast same as palette or something better (animation?)
  // maybe the overlay could animate between semi-transparent black and white
  return qGray(color) < 128 ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
}

template <typename Pixel>
class WandPolicy {
public:
  WandPolicy(
    gfx::Surface<PixelRgba> overlay,
    gfx::Surface<PixelMask> mask,
    gfx::CSurface<Pixel> source,
    const PixelRgba constrastColor
  ) : overlay{overlay},
      mask{mask},
      source{source},
      constrastColor{constrastColor} {}

  bool start(const gfx::Point pos) {
    startColor = source.ref(pos);
    maskCheckColor = mask.ref(pos);
    maskColor = ~maskCheckColor;
    if (maskColor == 0) {
      overlayColor = qRgba(0, 0, 0, 0);
    } else {
      overlayColor = constrastColor;
    }
    return true;
  }
  
  gfx::Size size() const {
    return source.size();
  }
  
  bool check(const gfx::Point pos) const {
    return source.ref(pos) == startColor &&
           mask.ref(pos) == maskCheckColor;
  }
  
  void set(const gfx::Point pos) const {
    overlay.ref(pos) = overlayColor;
    mask.ref(pos) = maskColor;
  }

private:
  gfx::Surface<PixelRgba> overlay;
  gfx::Surface<PixelMask> mask;
  gfx::CSurface<Pixel> source;
  Pixel startColor;
  PixelRgba constrastColor;
  PixelRgba overlayColor;
  PixelMask maskColor;
  PixelMask maskCheckColor;
};

}

void WandSelectTool::addToSelection(const ToolMouseEvent &event) {
  QRect rect = toRect(ctx->size);
  if (sampleCell(*ctx->cell, event.pos) == 0) {
    ctx->requireCell(rect);
  } else {
    rect = rect.intersected(ctx->cell->rect());
  }
  const gfx::Surface overlaySurface = makeSurface<PixelRgba>(*ctx->overlay).view(convert(rect));
  const gfx::Surface maskSurface = makeSurface<PixelMask>(mask).view(convert(rect));
  const gfx::Point cellPos = convert(event.pos - rect.topLeft());
  const gfx::Rect cellRect = convert(rect.translated(-ctx->cell->pos));

  switch (ctx->format) {
    case Format::rgba: {
      gfx::Surface surface = makeCSurface<PixelRgba>(ctx->cell->img).view(cellRect);
      WandPolicy policy{
        overlaySurface,
        maskSurface,
        surface,
        contrastColor(surface.ref(cellPos))
      };
      bounds = bounds.united(convert(gfx::floodFill(policy, cellPos)).translated(rect.topLeft()));
      break;
    }
    case Format::index: {
      gfx::Surface surface = makeCSurface<PixelIndex>(ctx->cell->img).view(cellRect);
      WandPolicy policy{
        overlaySurface,
        maskSurface,
        surface,
        contrastColor(ctx->palette[surface.ref(cellPos)])
      };
      bounds = bounds.united(convert(gfx::floodFill(policy, cellPos)).translated(rect.topLeft()));
      break;
    }
    case Format::gray: {
      gfx::Surface surface = makeCSurface<PixelGray>(ctx->cell->img).view(cellRect);
      WandPolicy policy{
        overlaySurface,
        maskSurface,
        surface,
        qRgba(0, 0, scaleOverlayGray(surface.ref(cellPos)), scaleOverlayAlpha(255))
      };
      bounds = bounds.united(convert(gfx::floodFill(policy, cellPos)).translated(rect.topLeft()));
      break;
    }
  }
}
