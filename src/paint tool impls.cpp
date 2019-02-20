//
//  paint tool impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "paint tool impls.hpp"

#include <cmath>
#include "cell impls.hpp"
#include <QtGui/qpainter.h>

namespace {

void initPainter(QPainter &painter, SourceCell *source) {
  assert(source);
  painter.begin(&source->image.data);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.resetTransform();
  applyInvTransform(painter, source->image);
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
const QPen round_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin
};
const QPen square_pen{
  Qt::NoBrush, 1.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin
};

void clearOverlay(QImage *overlay) {
  assert(overlay);
  overlay->fill(0);
}

void drawPointOverlay(QImage *overlay, const QPoint pos, QPen colorPen) {
  assert(overlay);
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
  : pen{round_pen} {}

bool BrushTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

ToolChanges BrushTool::mouseDown(const ToolEvent &event) {
  assert(source);
  if (button != ButtonType::none) return ToolChanges::none;
  clearOverlay(event.overlay);
  drawPointOverlay(event.overlay, event.pos, pen);
  button = event.type;
  lastPos = event.pos;
  pen.setColor(toColor(selectColor(event.colors, event.type)));
  QPainter painter;
  initPainter(painter, source);
  painter.setPen(pen);
  painter.drawPoint(lastPos);
  return ToolChanges::cell_overlay;
}

ToolChanges BrushTool::mouseMove(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  clearOverlay(event.overlay);
  drawPointOverlay(event.overlay, event.pos, pen);
  if (event.type == ButtonType::none) return ToolChanges::overlay;
  QPainter painter;
  initPainter(painter, source);
  painter.setPen(pen);
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
  initPainter(painter, source);
  painter.setPen(pen);
  painter.drawLine(lastPos, event.pos);
  return ToolChanges::cell_overlay;
}

void BrushTool::setDiameter(const int diameter) {
  assert(min_thickness <= diameter && diameter <= max_thickness);
  pen.setWidth(diameter);
}

int BrushTool::getDiameter() const {
  return pen.width();
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
ToolChanges DragPaintTool<Derived>::mouseDown(const ToolEvent &event) {
  assert(source);
  if (button != ButtonType::none) return ToolChanges::none;
  clearOverlay(event.overlay);
  that()->drawOverlay(event.overlay, event.pos);
  button = event.type;
  startPos = lastPos = event.pos;
  copyImage(cleanImage, source->image.data);
  that()->setColor(toColor(selectColor(event.colors, event.type)));
  QPainter painter;
  initPainter(painter, source);
  that()->setupPainter(painter);
  that()->drawPoint(painter, startPos);
  return ToolChanges::cell_overlay;
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseMove(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  clearOverlay(event.overlay);
  that()->drawOverlay(event.overlay, event.pos);
  if (event.type == ButtonType::none) return ToolChanges::overlay;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  QPainter painter;
  initPainter(painter, source);
  that()->setupPainter(painter);
  that()->drawDrag(painter, startPos, lastPos);
  return ToolChanges::cell_overlay;
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseUp(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  if (event.type != button) return ToolChanges::none;
  clearOverlay(event.overlay);
  button = ButtonType::none;
  lastPos = event.pos;
  copyImage(source->image.data, cleanImage);
  QPainter painter;
  initPainter(painter, source);
  that()->setupPainter(painter);
  that()->drawDrag(painter, startPos, lastPos);
  return ToolChanges::cell_overlay;
}

template <typename Derived>
Derived *DragPaintTool<Derived>::that() {
  return static_cast<Derived *>(this);
}

LineTool::LineTool()
  : pen{round_pen} {}

LineTool::~LineTool() = default;

void LineTool::setThickness(const int thickness) {
  assert(min_thickness <= thickness && thickness <= max_thickness);
  pen.setWidth(thickness);
}

int LineTool::getThickness() const {
  return pen.width();
}

void LineTool::setColor(const QColor color) {
  pen.setColor(color);
}

void LineTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
}

void LineTool::drawPoint(QPainter &painter, const QPoint pos) {
  painter.drawPoint(pos);
}

void LineTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  painter.drawLine(start, end);
}

void LineTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}

StrokedCircleTool::StrokedCircleTool()
  : pen{round_pen} {}

StrokedCircleTool::~StrokedCircleTool() = default;

void StrokedCircleTool::setThickness(const int thickness) {
  assert(min_thickness <= thickness && thickness <= max_thickness);
  pen.setWidth(thickness);
}

int StrokedCircleTool::getThickness() const {
  return pen.width();
}

void StrokedCircleTool::setCenter(const CircleCenter cent) {
  center = cent;
}

CircleCenter StrokedCircleTool::getCenter() const {
  return center;
}

void StrokedCircleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void StrokedCircleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
}

void StrokedCircleTool::drawPoint(QPainter &, QPoint) {}

namespace {

int addEllipseWidth(const CircleCenter center) {
  return (center == CircleCenter::c2x1 || center == CircleCenter::c2x2);
}

int addEllipseHeight(const CircleCenter center) {
  return (center == CircleCenter::c1x2 || center == CircleCenter::c2x2);
}

double distance(const QPoint a, const QPoint b) {
  const int dx = a.x() - b.x();
  const int dy = a.y() - b.y();
  return std::sqrt(dx*dx + dy*dy);
}

QRect calcEllipseRect(
  const CircleCenter center,
  const QPoint start,
  const QPoint end,
  const int thickness
) {
  // @TODO revisit this
  const int radius = std::round(distance(start, end));
  return QRect{
    start.x() - radius,
    start.y() - radius,
    radius * 2 + 1 + addEllipseWidth(center),
    radius * 2 + 1 + addEllipseHeight(center)
  };
  
  /*const double halfThick = thickness / 2.0;
  QRectF rect{
    start.x() - radius,
    start.y() - radius,
    radius * 2.0,
    radius * 2.0
  };
  QRectF rect{
    start.x() - radius + halfThick,
    start.y() - radius + halfThick,
    radius * 2.0 - thickness,
    radius * 2.0 - thickness
  };
  QRectF rect{7.0, 7.0, 24.0 - 7.0, 25.0 - 7.0};
  QRectF rect{7.0 + 1.0, 7.0 + 1.0, 17.0 - 2.0, 18.0 - 2.0};
  QRectF rect{7.0 + 0.5, 7.0 + 0.5, 17.0 - 1.0, 18.0 - 1.0};
  rect.setRight(rect.right() + addEllipseWidth(center));
  rect.setBottom(rect.bottom() + addEllipseHeight(center));
  return rect;*/
}

}

void StrokedCircleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  painter.drawEllipse(calcEllipseRect(center, start, end, pen.width()));
}

void StrokedCircleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}

FilledCircleTool::FilledCircleTool()
  : pen{round_pen} {}

FilledCircleTool::~FilledCircleTool() = default;

void FilledCircleTool::setCenter(const CircleCenter cent) {
  center = cent;
}

CircleCenter FilledCircleTool::getCenter() const {
  return center;
}

void FilledCircleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void FilledCircleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
  painter.setBrush(pen.color());
}

void FilledCircleTool::drawPoint(QPainter &, QPoint) {}

void FilledCircleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  painter.drawEllipse(calcEllipseRect(center, start, end, 1));
}

void FilledCircleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}

StrokedRectangleTool::StrokedRectangleTool()
  : pen{square_pen} {}

StrokedRectangleTool::~StrokedRectangleTool() = default;

void StrokedRectangleTool::setThickness(const int thickness) {
  pen.setWidth(thickness);
}

int StrokedRectangleTool::getThickness() const {
  return pen.width();
}

void StrokedRectangleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void StrokedRectangleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
}

void StrokedRectangleTool::drawPoint(QPainter &painter, const QPoint pos) {
  painter.drawPoint(pos);
}

namespace {

QRect calcRect(const QPoint start, const QPoint end) {
  return QRect{
    std::min(start.x(), end.x()),
    std::min(start.y(), end.y()),
    std::abs(start.x() - end.x()),
    std::abs(start.y() - end.y())
  };
}

}

void StrokedRectangleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  painter.drawRect(calcRect(start, end));
}

void StrokedRectangleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}

FilledRectangleTool::FilledRectangleTool()
  : pen{square_pen} {}

FilledRectangleTool::~FilledRectangleTool() = default;

void FilledRectangleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void FilledRectangleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
  painter.setBrush(pen.color());
}

void FilledRectangleTool::drawPoint(QPainter &painter, const QPoint pos) {
  painter.drawPoint(pos);
}

void FilledRectangleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  painter.drawRect(calcRect(start, end));
}

void FilledRectangleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}
