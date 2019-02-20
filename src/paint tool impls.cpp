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
  painter.setTransform(getInvTransform(source->image));
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
  // the QRgb constructor sets alpha to 255 for some reason
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

void clearImage(QImage &dst) {
  dst.detach();
  std::memset(dst.bits(), 0, dst.sizeInBytes());
}

void clearOverlay(QImage *overlay) {
  assert(overlay);
  clearImage(*overlay);
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

bool FloodFillTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

namespace {

template <typename Pixel>
Pixel *pixelAddr(uchar *bits, const int bbl, const QPoint pos) noexcept {
  return reinterpret_cast<Pixel *>(bits + bbl * pos.y()) + pos.x();
}

template <typename Pixel>
class PixelGetter {
public:
  PixelGetter(QImage &img, const Pixel startColor, const Pixel toolColor)
    : bits{img.bits()},
      bbl{img.bytesPerLine()},
      startColor{startColor},
      toolColor{toolColor} {}
  
  bool filled(const QPoint pos) const noexcept {
    return *pixelAddr<Pixel>(bits, bbl, pos) != startColor;
  }
  void fill(const QPoint pos) noexcept {
    *pixelAddr<Pixel>(bits, bbl, pos) = toolColor;
  }

private:
  uchar *const bits;
  const int bbl;
  Pixel startColor;
  Pixel toolColor;
};

QPoint up(const QPoint p) {
  return {p.x(), p.y() - 1};
}

QPoint right(const QPoint p) {
  return {p.x() + 1, p.y()};
}

QPoint down(const QPoint p) {
  return {p.x(), p.y() + 1};
}

QPoint left(const QPoint p) {
  return {p.x() - 1, p.y()};
}

// Flood Fill algorithm by Adam Milazzo
// http://www.adammil.net/blog/v126_A_More_Efficient_Flood_Fill.html

template <typename Pixel>
void startFloodFill(PixelGetter<Pixel>, QPoint, QSize);

template <typename Pixel>
void floodFillImpl(PixelGetter<Pixel> px, QPoint pos, const QSize size) {
  int lastRowLength = 0;
  do {
    int rowLength = 0;
    QPoint start = pos;
    if (lastRowLength != 0 && px.filled(pos)) {
      do {
        if (--lastRowLength == 0) return;
        pos = right(pos);
      } while (px.filled(pos));
    } else {
      while (pos.x() != 0 && !px.filled(left(pos))) {
        pos = left(pos);
        px.fill(pos);
        if (pos.y() != 0 && !px.filled(up(pos))) startFloodFill(px, up(pos), size);
        ++rowLength;
        ++lastRowLength;
      }
    }
    while (start.x() < size.width() && !px.filled(start)) {
      px.fill(start);
      start = right(start);
      ++rowLength;
    }
    if (rowLength < lastRowLength) {
      const int endX = pos.x() + lastRowLength;
      while (++start.rx() < endX) {
        if (!px.filled(start)) floodFillImpl(px, start, size);
      }
    } else if (rowLength > lastRowLength && pos.y() != 0) {
      QPoint above = up({pos.x() + lastRowLength, pos.y()});
      while (++above.rx() < start.x()) {
        if (!px.filled(above)) startFloodFill(px, above, size);
      }
    }
    lastRowLength = rowLength;
    pos = down(pos);
  } while (lastRowLength != 0 && pos.y() < size.height());
}

template <typename Pixel>
void startFloodFill(PixelGetter<Pixel> px, QPoint pos, const QSize size) {
  while (true) {
    const QPoint startPos = pos;
    while (pos.y() != 0 && !px.filled(up(pos))) pos = up(pos);
    while (pos.x() != 0 && !px.filled(left(pos))) pos = left(pos);
    if (pos == startPos) break;
  }
  floodFillImpl(px, pos, size);
}

template <typename Pixel>
ToolChanges floodFill(QImage &img, const QPoint startPos, const QRgb color) {
  const Pixel toolColor = static_cast<Pixel>(color);
  const Pixel startColor = *pixelAddr<Pixel>(img.bits(), img.bytesPerLine(), startPos);
  if (startColor == toolColor) return ToolChanges::overlay;
  PixelGetter<Pixel> px{img, startColor, toolColor};
  startFloodFill(px, startPos, img.size());
  return ToolChanges::cell_overlay;
}

ToolChanges floodFill(QImage &img, const QPoint pos, const QRgb color) {
  if (img.depth() == 8) {
    return floodFill<uint8_t>(img, pos, color);
  } else if (img.depth() == 32) {
    return floodFill<uint32_t>(img, pos, color);
  } else {
    Q_UNREACHABLE();
  }
}

}

ToolChanges FloodFillTool::mouseDown(const ToolEvent &event) {
  assert(source);
  lastPos = event.pos;
  clearOverlay(event.overlay);
  drawPointOverlay(event.overlay, event.pos, square_pen);
  const QRgb toolColor = selectColor(event.colors, event.type);
  const QTransform xform = getInvTransform(source->image);
  const ToolChanges changes = floodFill(source->image.data, xform.map(event.pos), toolColor);
  return changes;
}

ToolChanges FloodFillTool::mouseMove(const ToolEvent &event) {
  assert(source);
  if (event.pos == lastPos) return ToolChanges::none;
  lastPos = event.pos;
  clearOverlay(event.overlay);
  drawPointOverlay(event.overlay, event.pos, square_pen);
  return ToolChanges::overlay;
}

ToolChanges FloodFillTool::mouseUp(const ToolEvent &) {
  assert(source);
  return ToolChanges::none;
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
  startPos = no_point;
  return ToolChanges::cell_overlay;
}

template <typename Derived>
bool DragPaintTool<Derived>::isDragging() const {
  return startPos != no_point;
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

int StrokedCircleTool::getRadius() const {
  return isDragging() ? radius : no_radius;
}

void StrokedCircleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void StrokedCircleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
}

void StrokedCircleTool::drawPoint(QPainter &, QPoint) {
  radius = 0;
}

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

int calcRadius(const QPoint start, const QPoint end) {
  return std::round(distance(start, end));
}

QRect calcEllipseRect(
  const CircleCenter center,
  const QPoint start,
  const int radius,
  const int thickness
) {
  // @TODO revisit this
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
  radius = calcRadius(start, end);
  painter.drawEllipse(calcEllipseRect(center, start, radius, pen.width()));
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

int FilledCircleTool::getRadius() const {
  return isDragging() ? radius : no_radius;
}

void FilledCircleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void FilledCircleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
  painter.setBrush(pen.color());
}

void FilledCircleTool::drawPoint(QPainter &, QPoint) {
  radius = 0;
}

void FilledCircleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  radius = calcRadius(start, end);
  painter.drawEllipse(calcEllipseRect(center, start, radius, 1));
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

QSize StrokedRectangleTool::getSize() const {
  return isDragging() ? size : no_size;
}

void StrokedRectangleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void StrokedRectangleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
}

void StrokedRectangleTool::drawPoint(QPainter &painter, const QPoint pos) {
  size = QSize{1, 1};
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
  const QRect rect = calcRect(start, end);
  size = rect.size() + QSize{1, 1};
  painter.drawRect(rect);
}

void StrokedRectangleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}

FilledRectangleTool::FilledRectangleTool()
  : pen{square_pen} {}

FilledRectangleTool::~FilledRectangleTool() = default;

QSize FilledRectangleTool::getSize() const {
  return isDragging() ? size : no_size;
}

void FilledRectangleTool::setColor(const QColor color) {
  pen.setColor(color);
}

void FilledRectangleTool::setupPainter(QPainter &painter) {
  painter.setPen(pen);
  painter.setBrush(pen.color());
}

void FilledRectangleTool::drawPoint(QPainter &painter, const QPoint pos) {
  size = QSize{1, 1};
  painter.drawPoint(pos);
}

void FilledRectangleTool::drawDrag(QPainter &painter, const QPoint start, const QPoint end) {
  const QRect rect = calcRect(start, end);
  size = rect.size() + QSize{1, 1};
  painter.drawRect(rect);
}

void FilledRectangleTool::drawOverlay(QImage *overlay, const QPoint pos) {
  drawPointOverlay(overlay, pos, pen);
}
