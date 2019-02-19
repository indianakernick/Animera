//
//  paint tool impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "paint tool impls.hpp"

#include "cell impls.hpp"
#include <QtGui/qpainter.h>

namespace {

void initPainter(QPainter &painter, SourceCell *source, const QPen &pen) {
  assert(source);
  painter.begin(&source->image.data);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.resetTransform();
  applyInvTransform(painter, source->image);
  painter.setPen(pen);
}

QRgb selectColor(const ToolColors &colors, const ButtonType button) {
  switch (button) {
    case ButtonType::primary: return colors.primary;
    case ButtonType::secondary: return colors.secondary;
    case ButtonType::erase: return colors.erase;
    default: Q_UNREACHABLE();
  }
  return 0;
}

QColor toColor(const QRgb rgba) {
  // the QRgb constructor just sets alpha to 255 for some reason
  return QColor{qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba)};
}

const QColor overlay_color{
  overlay_gray, overlay_gray, overlay_gray, overlay_alpha
};

}

BrushTool::BrushTool()
  : pen{Qt::NoBrush, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin} {}

bool BrushTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

ToolChanges BrushTool::mouseDown(const ToolEvent &event) {
  assert(source);
  if (button != ButtonType::none) return ToolChanges::none;
  drawOverlay(event.overlay, event.pos);
  button = event.type;
  lastPos = event.pos;
  pen.setColor(toColor(selectColor(event.colors, event.type)));
  QPainter painter;
  initPainter(painter, source, pen);
  painter.drawPoint(lastPos);
  return ToolChanges::cell_overlay;
}

ToolChanges BrushTool::mouseMove(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  drawOverlay(event.overlay, event.pos);
  if (event.type == ButtonType::none) return ToolChanges::overlay;
  QPainter painter;
  initPainter(painter, source, pen);
  painter.drawLine(lastPos, event.pos);
  lastPos = event.pos;
  return ToolChanges::cell_overlay;
}

ToolChanges BrushTool::mouseUp(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  if (event.type == button) {
    clearOverlay(event.overlay);
    button = ButtonType::none;
    QPainter painter;
    initPainter(painter, source, pen);
    painter.drawLine(lastPos, event.pos);
    return ToolChanges::cell_overlay;
  } else {
    return ToolChanges::none;
  }
}

void BrushTool::setDiameter(const int diameter) {
  assert(min_diameter <= diameter && diameter <= max_diameter);
  pen.setWidth(diameter);
}

int BrushTool::getDiameter() const {
  return pen.width();
}

QPen BrushTool::overlayPen() const {
  QPen newPen = pen;
  newPen.setColor(overlay_color);
  return newPen;
}

void BrushTool::clearOverlay(QImage *overlay) {
  assert(overlay);
  overlay->fill(0);
}

void BrushTool::drawOverlay(QImage *overlay, const QPoint pos) {
  clearOverlay(overlay);
  QPainter painter{overlay};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.setPen(overlayPen());
  painter.drawPoint(pos);
}
