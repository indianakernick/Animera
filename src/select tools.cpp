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
  if (event.button == ButtonType::secondary && !overlay.isNull()) {
    mode = opposite(mode);
  }
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
    status.appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    blitImage(*ctx->overlay, overlay, event.pos + offset);
    status.appendLabeled({event.pos + offset, selection.size()});
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) startPos = event.pos;
    ctx->emitChanges(ToolChanges::overlay);
  } else if (mode == SelectMode::paste) {
    const QRect rect{event.pos + offset, selection.size()};
    if (event.button == ButtonType::primary) {
      ctx->requireCell(rect);
      blitImage(ctx->cell->image, selection, rect.topLeft() - ctx->cell->image.offset());
    } else if (event.button == ButtonType::erase) {
      ctx->requireCell(rect);
      drawFilledRect(ctx->cell->image, ctx->colors.erase, rect);
    } else {
      return ctx->emitChanges(ToolChanges::overlay);
    }
    ctx->emitChanges(ToolChanges::cell_overlay);
    ctx->finishChange();
  } else Q_UNREACHABLE();
}

void RectangleSelectTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  StatusMsg status;
  status.appendLabeled(mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      const QRect rect = unite(startPos, event.pos);
      drawStrokedRect(*ctx->overlay, tool_overlay_color, rect);
      status.appendLabeled(rect);
    } else {
      drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
      status.appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    blitImage(*ctx->overlay, overlay, event.pos + offset);
    status.appendLabeled({event.pos + offset, selection.size()});
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void RectangleSelectTool::mouseUp(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary) return;
  clearImage(*ctx->overlay);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos); // why?
    const QRect rect = unite(startPos, event.pos);
    overlay = QImage{rect.size(), qimageFormat(Format::rgba)};
    if (ctx->cell->image.isNull()) {
      selection = {rect.size(), qimageFormat(ctx->format)};
      clearImage(selection);
      writeOverlay(ctx->palette, ctx->format, overlay);
    } else {
      const QRect offsetRect = rect.translated(-ctx->cell->image.offset());
      selection = blitImage(ctx->cell->image, offsetRect);
      writeOverlay(ctx->palette, ctx->format, overlay, selection);
    }
    offset = rect.topLeft() - event.pos;
    mode = SelectMode::paste;
  }
  blitImage(*ctx->overlay, overlay, event.pos + offset);
  StatusMsg status;
  status.appendLabeled(mode);
  status.appendLabeled({event.pos + offset, selection.size()});
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
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
  if (event.button == ButtonType::secondary && !overlay.isNull()) {
    mode = opposite(mode);
  }
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
    status.appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    blitImage(*ctx->overlay, overlay, event.pos + offset);
    status.appendLabeled({event.pos + offset, selection.size()});
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) polygon.init(event.pos);
    ctx->emitChanges(ToolChanges::overlay);
  } else if (mode == SelectMode::paste) {
    const QRect rect{event.pos + offset, selection.size()};
    if (event.button == ButtonType::primary) {
      ctx->requireCell(rect);
      const QPoint offsetPos = rect.topLeft() - ctx->cell->image.offset();
      blitMaskImage(ctx->cell->image, mask, selection, offsetPos);
    } else if (event.button == ButtonType::erase) {
      ctx->requireCell(rect);
      const QPoint offsetPos = rect.topLeft() - ctx->cell->image.offset();
      fillMaskImage(ctx->cell->image, mask, ctx->colors.erase, offsetPos);
    } else {
      return ctx->emitChanges(ToolChanges::overlay);
    }
    ctx->emitChanges(ToolChanges::cell_overlay);
    ctx->finishChange();
  } else Q_UNREACHABLE();
}

void PolygonSelectTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  StatusMsg status;
  status.appendLabeled(mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      polygon.push(event.pos);
      drawFilledPolygon(*ctx->overlay, tool_overlay_color, polygon, QPoint{0, 0});
      status.appendLabeled(polygon.bounds());
    } else {
      drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
      status.appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    blitImage(*ctx->overlay, overlay, event.pos + offset);
    status.appendLabeled({event.pos + offset, selection.size()});
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void PolygonSelectTool::mouseUp(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary) return;
  clearImage(*ctx->overlay);
  if (mode == SelectMode::copy) {
    polygon.push(event.pos);
    const QRect bounds = polygon.bounds();
    mask = QImage{bounds.size(), mask_format};
    clearImage(mask);
    drawFilledPolygon(mask, mask_color_on, polygon, -bounds.topLeft());
    if (ctx->cell->image.isNull()) {
      selection = {bounds.size(), qimageFormat(ctx->format)};
      clearImage(selection);
    } else {
      const QPoint offsetPos = bounds.topLeft() - ctx->cell->image.offset();
      selection = blitMaskImage(ctx->cell->image, mask, offsetPos);
    }
    overlay = QImage{bounds.size(), qimageFormat(Format::rgba)};
    writeOverlay(ctx->palette, ctx->format, overlay, selection, mask);
    offset = bounds.topLeft() - event.pos;
    mode = SelectMode::paste;
  }
  blitImage(*ctx->overlay, overlay, event.pos + offset);
  StatusMsg status;
  status.appendLabeled(mode);
  status.appendLabeled({event.pos + offset, selection.size()});
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void WandSelectTool::attachCell() {
  mode = SelectMode::copy;
  const QImage::Format format = qimageFormat(ctx->format);
  if (selection.format() != format || selection.size() != ctx->size) {
    selection = {ctx->size, format};
  }
  if (overlay.size() != ctx->size) {
    overlay = {ctx->size, qimageFormat(Format::rgba)};
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
    status.appendLabeled(event.pos + offset);
    clearImage(*ctx->overlay);
    blitImage(*ctx->overlay, overlay, event.pos + offset);
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);

  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      addToSelection(event);
    }
    ctx->emitChanges(ToolChanges::overlay);
  } else if (mode == SelectMode::paste) {
    // TODO: this is very similar to PolygonSelectTool
    const QRect rect{event.pos + offset, selection.size()};
    if (event.button == ButtonType::primary) {
      ctx->requireCell(rect);
      const QPoint offsetPos = rect.topLeft() - ctx->cell->image.offset();
      blitMaskImage(ctx->cell->image, mask, selection, offsetPos);
    } else if (event.button == ButtonType::erase) {
      ctx->requireCell(rect);
      const QPoint offsetPos = rect.topLeft() - ctx->cell->image.offset();
      fillMaskImage(ctx->cell->image, mask, ctx->colors.erase, offsetPos);
    } else {
      return ctx->emitChanges(ToolChanges::overlay);
    }
    ctx->emitChanges(ToolChanges::cell_overlay);
    ctx->finishChange();
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
    blitImage(*ctx->overlay, overlay, event.pos + offset);
    ctx->emitChanges(ToolChanges::overlay);
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
}

void WandSelectTool::mouseUp(const ToolMouseEvent &) {}

void WandSelectTool::toggleMode(const ToolMouseEvent &event) {
  mode = opposite(mode);
  if (mode == SelectMode::copy) {
    clearImage(*ctx->overlay);
    clearImage(overlay);
    clearImage(selection);
    clearImage(mask);
  } else if (mode == SelectMode::paste) {
    selection = blitMaskImage(ctx->cell->image, mask, -ctx->cell->pos());
    writeOverlay(ctx->palette, ctx->format, overlay, selection, mask);
    offset = -event.pos;
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
  const gfx::Rect cellRect = convert(rect.translated(-ctx->cell->pos()));

  switch (ctx->format) {
    case Format::rgba: {
      gfx::Surface surface = makeCSurface<PixelRgba>(ctx->cell->image).view(cellRect);
      WandPolicy policy{
        overlaySurface,
        maskSurface,
        surface,
        contrastColor(surface.ref(cellPos))
      };
      gfx::floodFill(policy, cellPos);
      break;
    }
    case Format::index: {
      gfx::Surface surface = makeCSurface<PixelIndex>(ctx->cell->image).view(cellRect);
      WandPolicy policy{
        overlaySurface,
        maskSurface,
        surface,
        contrastColor(ctx->palette[surface.ref(cellPos)])
      };
      gfx::floodFill(policy, cellPos);
      break;
    }
    case Format::gray: {
      gfx::Surface surface = makeCSurface<PixelGray>(ctx->cell->image).view(cellRect);
      WandPolicy policy{
        overlaySurface,
        maskSurface,
        surface,
        qRgba(0, 0, scaleOverlayGray(surface.ref(cellPos)), scaleOverlayAlpha(255))
      };
      gfx::floodFill(policy, cellPos);
      break;
    }
  }
  
  // TODO: get the rectangle of the flood fill and only paste that rectangle
}
