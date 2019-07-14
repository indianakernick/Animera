//
//  select tools.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "select tools.hpp"

#include "cell.hpp"
#include "masking.hpp"
#include "painting.hpp"
#include "composite.hpp"
#include "flood fill.hpp"
#include "surface factory.hpp"

void RectangleSelectTool::detachCell() {
  ctx->clearStatus();
}

void RectangleSelectTool::mouseLeave(const ToolLeaveEvent &) {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void RectangleSelectTool::mouseDown(const ToolMouseEvent &event) {
  ctx->requireCell();
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
    status.appendLabeled({event.pos + offset, overlay.size()});
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) startPos = event.pos;
    ctx->emitChanges(ToolChanges::overlay);
  } else if (mode == SelectMode::paste) {
    if (event.button == ButtonType::primary) {
      blitImage(ctx->cell->image, selection, event.pos + offset);
    } else if (event.button == ButtonType::erase) {
      const QRect rect{event.pos + offset, selection.size()};
      drawFilledRect(ctx->cell->image, ctx->colors.erase, rect);
    } else {
      return ctx->emitChanges(ToolChanges::overlay);
    }
    ctx->finishChange();
    ctx->emitChanges(ToolChanges::cell_overlay);
  } else Q_UNREACHABLE();
}

void RectangleSelectTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  StatusMsg status;
  status.appendLabeled(mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      const QRect rect = QRect{startPos, event.pos}.normalized();
      drawStrokedRect(*ctx->overlay, tool_overlay_color, rect, 1);
      status.appendLabeled(rect);
    } else {
      drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
      status.appendLabeled(event.pos);
    }
  } else if (mode == SelectMode::paste) {
    blitImage(*ctx->overlay, overlay, event.pos + offset);
    status.appendLabeled({event.pos + offset, overlay.size()});
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void RectangleSelectTool::mouseUp(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary) return;
  clearImage(*ctx->overlay);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
    const QRect rect = QRect{startPos, event.pos}.normalized();
    selection = blitImage(ctx->cell->image, rect);
    overlay = QImage{selection.size(), qimageFormat(Format::rgba)};
    writeOverlay(ctx->palette, ctx->format, overlay, selection);
    offset = rect.topLeft() - event.pos;
    mode = SelectMode::paste;
  }
  blitImage(*ctx->overlay, overlay, event.pos + offset);
  StatusMsg status;
  status.appendLabeled(mode);
  status.appendLabeled({event.pos + offset, overlay.size()});
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
  ctx->requireCell();
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
    status.appendLabeled({event.pos + offset, overlay.size()});
  } else Q_UNREACHABLE();
  
  ctx->showStatus(status);
  
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) polygon.init(event.pos);
    ctx->emitChanges(ToolChanges::overlay);
  } else if (mode == SelectMode::paste) {
    if (event.button == ButtonType::primary) {
      blitMaskImage(ctx->cell->image, mask, selection, event.pos + offset);
    } else if (event.button == ButtonType::erase) {
      // @TODO should this be encapsulated in another file?
      visitSurface(ctx->cell->image, ctx->colors.erase, [this, &event](auto surface, auto color) {
        maskFillRegion(surface, makeCSurface<uint8_t>(mask), color, event.pos + offset);
      });
    } else {
      return ctx->emitChanges(ToolChanges::overlay);
    }
    ctx->finishChange();
    ctx->emitChanges(ToolChanges::cell_overlay);
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
    status.appendLabeled({event.pos + offset, overlay.size()});
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void PolygonSelectTool::mouseUp(const ToolMouseEvent &event) {
  if (event.button != ButtonType::primary) return;
  clearImage(*ctx->overlay);
  QPolygon p;
  if (mode == SelectMode::copy) {
    polygon.push(event.pos);
    const QRect clippedBounds = polygon.bounds().intersected(ctx->cell->image.rect());
    mask = QImage{clippedBounds.size(), mask_format};
    clearImage(mask);
    drawFilledPolygon(mask, mask_color_on, polygon, -clippedBounds.topLeft());
    selection = blitMaskImage(ctx->cell->image, mask, clippedBounds.topLeft());
    overlay = QImage{selection.size(), qimageFormat(Format::rgba)};
    writeOverlay(ctx->palette, ctx->format, overlay, selection, mask);
    offset = clippedBounds.topLeft() - event.pos;
    mode = SelectMode::paste;
  }
  blitImage(*ctx->overlay, overlay, event.pos + offset);
  StatusMsg status;
  status.appendLabeled(mode);
  status.appendLabeled({event.pos + offset, overlay.size()});
  ctx->showStatus(status);
  ctx->emitChanges(ToolChanges::overlay);
}

void WandSelectTool::attachCell() {
  mode = SelectMode::copy;
  if (!compatible(ctx->cell->image, selection)) {
    selection = makeCompatible(ctx->cell->image);
    overlay = QImage{selection.size(), qimageFormat(Format::rgba)};
    mask = makeMask(selection.size());
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
  ctx->requireCell();
  if (event.button == ButtonType::secondary) {
    toggleMode(event);
  }
  StatusMsg status;
  status.appendLabeled(mode);
  
  if (mode == SelectMode::copy) {
    status.appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    status.appendLabeled({event.pos + offset, selection.size()});
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
    // @TODO this is very similar to PolygonSelectTool
    if (event.button == ButtonType::primary) {
      blitMaskImage(ctx->cell->image, mask, selection, event.pos + offset);
    } else if (event.button == ButtonType::erase) {
      visitSurface(ctx->cell->image, ctx->colors.erase, [this, &event](auto surface, auto color) {
        maskFillRegion(surface, makeCSurface<uint8_t>(mask), color, event.pos + offset);
      });
    } else {
      return ctx->emitChanges(ToolChanges::overlay);
    }
    ctx->finishChange();
    ctx->emitChanges(ToolChanges::cell_overlay);
  } else Q_UNREACHABLE();
}

void WandSelectTool::mouseMove(const ToolMouseEvent &event) {
  StatusMsg status;
  status.appendLabeled(mode);
  if (mode == SelectMode::copy) {
    status.appendLabeled(event.pos);
  } else if (mode == SelectMode::paste) {
    status.appendLabeled({event.pos + offset, selection.size()});
    clearImage(*ctx->overlay);
    blitImage(*ctx->overlay, overlay, event.pos + offset);
    ctx->emitChanges(ToolChanges::overlay);
  } else Q_UNREACHABLE();
  ctx->showStatus(status);
}

void WandSelectTool::mouseUp(const ToolMouseEvent &) {
  
}

void WandSelectTool::toggleMode(const ToolMouseEvent &event) {
  mode = opposite(mode);
  if (mode == SelectMode::copy) {
    clearImage(*ctx->overlay);
    clearImage(overlay);
    clearImage(selection);
    clearImage(mask);
  } else if (mode == SelectMode::paste) {
    selection = blitMaskImage(ctx->cell->image, mask, {0, 0});
    writeOverlay(ctx->palette, ctx->format, overlay, selection, mask);
    offset = -event.pos;
  } else Q_UNREACHABLE();
}

namespace {

QRgb contrastColor(const QRgb color) {
  // @TODO constrast same as palette or something better (animation?)
  // maybe the overlay could animate between semi-transparent black and white
  return qGray(color) < 128 ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
}

template <typename Pixel>
class WandManip {
public:
  WandManip(Surface<PixelRgba> overlay, Surface<PixelMask> mask, CSurface<Pixel> source, const PixelRgba constrastColor)
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
  Surface<PixelRgba> overlay;
  Surface<PixelMask> mask;
  CSurface<Pixel> source;
  Pixel startColor;
  PixelRgba constrastColor;
  PixelRgba overlayColor;
  PixelMask maskColor;
  PixelMask maskCheckColor;
};

}

void WandSelectTool::addToSelection(const ToolMouseEvent &event) {
  switch (ctx->format) {
    case Format::rgba: {
      Surface surface = makeCSurface<PixelRgba>(ctx->cell->image);
      WandManip manip{
        makeSurface<PixelRgba>(*ctx->overlay),
        makeSurface<PixelMask>(mask),
        surface,
        contrastColor(surface.getPixel(event.pos))
      };
      floodFill(manip, event.pos);
      break;
    }
    case Format::palette: {
      Surface surface = makeCSurface<PixelPalette>(ctx->cell->image);
      WandManip manip{
        makeSurface<PixelRgba>(*ctx->overlay),
        makeSurface<PixelMask>(mask),
        surface,
        contrastColor(ctx->palette[surface.getPixel(event.pos)])
      };
      floodFill(manip, event.pos);
      break;
    }
    case Format::gray: {
      Surface surface = makeCSurface<PixelGray>(ctx->cell->image);
      WandManip manip{
        makeSurface<PixelRgba>(*ctx->overlay),
        makeSurface<PixelMask>(mask),
        surface,
        qRgba(0, 0, scaleOverlayGray(surface.getPixel(event.pos)), scaleOverlayAlpha(255))
      };
      floodFill(manip, event.pos);
      break;
    }
  }
}
