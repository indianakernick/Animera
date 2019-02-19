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
const QPen default_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin
};

void clearOverlay(QImage *overlay) {
  assert(overlay);
  overlay->fill(0);
}

void drawOverlay(QImage *overlay, const QPoint pos, QPen colorPen) {
  clearOverlay(overlay);
  QPainter painter{overlay};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setRenderHint(QPainter::Antialiasing, false);
  colorPen.setColor(overlay_color);
  painter.setPen(colorPen);
  painter.drawPoint(pos);
}

bool compatible(const QImage &a, const QImage &b) {
  return a.size() == b.size() && a.format() == b.format();
}

QImage makeCompatible(const QImage &img) {
  return QImage{img.size(), img.format()};
}

void copyImage(QImage &dst, const QImage &src) {
  assert(compatible(dst, src));
  dst.detach();
  std::memcpy(dst.bits(), src.constBits(), dst.sizeInBytes());
}

}

BrushTool::BrushTool()
  : pen{default_pen} {}

bool BrushTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

ToolChanges BrushTool::mouseDown(const ToolEvent &event) {
  assert(source);
  if (button != ButtonType::none) return ToolChanges::none;
  drawOverlay(event.overlay, event.pos, pen);
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
  drawOverlay(event.overlay, event.pos, pen);
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
  if (event.type != button) return ToolChanges::none;
  clearOverlay(event.overlay);
  button = ButtonType::none;
  QPainter painter;
  initPainter(painter, source, pen);
  painter.drawLine(lastPos, event.pos);
  return ToolChanges::cell_overlay;
}

void BrushTool::setDiameter(const int diameter) {
  assert(min_diameter <= diameter && diameter <= max_diameter);
  pen.setWidth(diameter);
}

int BrushTool::getDiameter() const {
  return pen.width();
}

LineTool::LineTool()
  : pen{default_pen} {}

bool LineTool::attachCell(Cell *cell) {
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

ToolChanges LineTool::mouseDown(const ToolEvent &event) {
  assert(source);
  if (button != ButtonType::none) return ToolChanges::none;
  drawOverlay(event.overlay, event.pos, pen);
  button = event.type;
  startPos = lastPos = event.pos;
  copyImage(cleanImage, source->image.data);
  pen.setColor(toColor(selectColor(event.colors, event.type)));
  QPainter painter;
  initPainter(painter, source, pen);
  painter.drawPoint(startPos);
  return ToolChanges::cell_overlay;
}

ToolChanges LineTool::mouseMove(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  drawOverlay(event.overlay, event.pos, pen);
  if (event.type == ButtonType::none) return ToolChanges::overlay;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  QPainter painter;
  initPainter(painter, source, pen);
  painter.drawLine(startPos, lastPos);
  return ToolChanges::cell_overlay;
}

ToolChanges LineTool::mouseUp(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  if (event.type != button) return ToolChanges::none;
  clearOverlay(event.overlay);
  button = ButtonType::none;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  QPainter painter;
  initPainter(painter, source, pen);
  painter.drawLine(startPos, lastPos);
  return ToolChanges::cell_overlay;
}

void LineTool::setThickness(const int thickness) {
  pen.setWidth(thickness);
}

int LineTool::getThickness() const {
  return pen.width();
}
