//
//  timeline cels widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 24/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "timeline cels widget.hpp"

#include "connect.hpp"
#include "painting.hpp"
#include <QtGui/qevent.h>
#include "group array.hpp"
#include <QtGui/qpainter.h>
#include "config colors.hpp"
#include "config geometry.hpp"

namespace {

void resizeImage(QImage &image, const QSize newSize) {
  image = QImage{newSize, QImage::Format_ARGB32_Premultiplied};
}

void resizeCopyImage(QImage &image, const QSize newSize) {
  QImage newImage{newSize, QImage::Format_ARGB32_Premultiplied};
  if (!newSize.isEmpty()) {
    QPainter painter{&newImage};
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(0, 0, image);
  }
  image = newImage;
}

}

CelsWidget::CelsWidget(QWidget *parent)
  : QWidget{parent},
    posImg{0, 0, QImage::Format_ARGB32_Premultiplied},
    selectionImg{0, 0, QImage::Format_ARGB32_Premultiplied},
    layersImg{0, 0, QImage::Format_ARGB32_Premultiplied} {
  setFocusPolicy(Qt::ClickFocus);
}

void CelsWidget::setSelection(const CelRect rect) {
  if (rect.minL <= rect.maxL && rect.minF <= rect.maxF) {
    selectionRect = {
      +rect.minF * cel_width,
      +rect.minL * cel_height,
      +(rect.maxF - rect.minF + FrameIdx{1}) * cel_width - glob_border_width,
      +(rect.maxL - rect.minL + LayerIdx{1}) * cel_height - glob_border_width
    };
  } else {
    selectionRect = {};
  }
  updateSelectionImg();
  update();
}

void CelsWidget::setPos(const CelPos pos) {
  const QPoint pixelPos = {+pos.f * cel_width, +pos.l * cel_height};
  Q_EMIT shouldEnsureVisible(pixelPos);
  posImg.fill(0);
  QPainter painter{&posImg};
  painter.setPen(Qt::NoPen);
  painter.setBrush(cel_pos_color);
  constexpr int size = 2 * cel_icon_pad + cel_icon_size;
  if (height() > cel_height || width() == cel_width) {
    painter.drawRect(0, +pos.l * cel_height, width(), size);
  }
  if (width() > cel_width) {
    painter.drawRect(+pos.f * cel_width, 0, size, height());
  }
  painter.end();
  update();
}

void CelsWidget::setLayerCels(const LayerIdx idx, tcb::span<const Cel> cels) {
  QPainter layers{&layersImg};
  layers.setCompositionMode(QPainter::CompositionMode_Source);
  QPainter borders{&bordersImg};
  borders.setCompositionMode(QPainter::CompositionMode_Source);
  
  celPainter.start(0, +idx * cel_height);
  
  layers.fillRect(0, celPainter.posY(), width(), cel_height, QColor{0, 0, 0, 0});
  borders.fillRect(0, celPainter.posY(), width(), cel_height, QColor{0, 0, 0, 0});
  
  for (const Cel &cel : cels) {
    if (*cel.cel) {
      celPainter.span(layers, cel.len);
      celPainter.span(borders, cel.len);
      celPainter.advance(cel.len);
      celPainter.border(borders);
    } else {
      for (FrameIdx f = {}; f != cel.len; ++f) {
        celPainter.advance();
        celPainter.border(borders);
      }
    }
  }
  
  borders.fillRect(
    0, celPainter.posY() + cel_height - glob_border_width,
    celPainter.posX() - cel_icon_pad, glob_border_width,
    glob_border_color
  );
  
  borders.end();
  layers.end();
  updateSelectionImg();
  update();
}

void CelsWidget::setFrameCount(const FrameIdx count) {
  setFixedWidth(+count * cel_width);
  resizeImage(posImg, size());
  resizeImage(selectionImg, size());
  resizeImage(layersImg, size());
  resizeImage(bordersImg, size());
  Q_EMIT resized();
}

void CelsWidget::setLayerCount(const LayerIdx count) {
  setFixedHeight(+count * cel_height);
  resizeImage(posImg, size());
  resizeImage(selectionImg, size());
  resizeCopyImage(layersImg, size());
  resizeCopyImage(bordersImg, size());
  Q_EMIT resized();
}

void CelsWidget::updateSelectionImg() {
  if (selectionRect.isEmpty()) {
    selectionImg.fill(0);
    return;
  }
  copyImage(selectionImg, layersImg);
  QPainter painter{&selectionImg};
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(selectionRect, cel_select_color);
  drawStrokedRect(
    selectionImg,
    PixelVar{cel_select_border_color.rgba()}, // TODO: QRgb
    selectionRect,
    glob_border_width
  );
}

void CelsWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.drawImage(0, 0, posImg);
  painter.drawImage(0, 0, layersImg);
  painter.drawImage(0, 0, bordersImg);
  painter.drawImage(0, 0, selectionImg);
}

void CelsWidget::focusOutEvent(QFocusEvent *) {
  Q_EMIT shouldClearSelection();
}

CelPos CelsWidget::getPos(QMouseEvent *event) {
  return {
    LayerIdx{std::clamp(event->pos().y(), 0, height() - 1) / cel_height},
    FrameIdx{std::clamp(event->pos().x(), 0, width() - 1) / cel_width}
  };
}

void CelsWidget::mousePressEvent(QMouseEvent *event) {
  Q_EMIT shouldSetPos(getPos(event));
  Q_EMIT shouldBeginSelection();
}

void CelsWidget::mouseMoveEvent(QMouseEvent *event) {
  Q_EMIT shouldSetPos(getPos(event));
  Q_EMIT shouldContinueSelection();
}

void CelsWidget::mouseReleaseEvent(QMouseEvent *event) {
  Q_EMIT shouldSetPos(getPos(event));
  Q_EMIT shouldEndSelection();
}

CelScrollWidget::CelScrollWidget(QWidget *parent)
  : ScrollAreaWidget{parent} {
  setFrameShape(NoFrame);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setStyleSheet("background-color:" + glob_main.name());
  setMinimumSize(cel_width + glob_scroll_width, cel_height + glob_scroll_width);
}

CelsWidget *CelScrollWidget::getChild() {
  auto *cels = new CelsWidget{this};
  rect = new QWidget{cels};
  rect->setVisible(false);
  CONNECT(cels, shouldEnsureVisible, this, ensureVisible);
  CONNECT(cels, resized,             this, changeMargins);
  setWidget(cels);
  return cels;
}

void CelScrollWidget::changeMargins() {
  Q_EMIT rightMarginChanged(rightMargin());
  Q_EMIT bottomMarginChanged(bottomMargin());
  adjustMargins();
}

void CelScrollWidget::ensureVisible(const QPoint pos) {
  const int border = bottomMargin() ? 0 : glob_border_width;
  rect->setGeometry(pos.x(), pos.y(), cel_width, cel_height - border);
  ensureWidgetVisible(rect, 0, 0);
}

void CelScrollWidget::resizeEvent(QResizeEvent *event) {
  changeMargins();
  ScrollAreaWidget::resizeEvent(event);
}

GroupsWidget::GroupsWidget(QWidget *parent)
  : QWidget{parent},
    groupImg{0, 0, QImage::Format_ARGB32_Premultiplied},
    selectionImg{0, 0, QImage::Format_ARGB32_Premultiplied} {
  setFixedSize(0, cel_height);
  setMouseTracking(true);
}

void GroupsWidget::setGroupArray(const tcb::span<const Group> groups) {
  groupArray = groups;
  
  groupImg.fill(0);
  QPainter painter{&groupImg};
  
  FrameIdx prevEnd = {};
  celPainter.start();
  
  for (const Group &group : groups) {
    const FrameIdx len = group.end - std::exchange(prevEnd, group.end);
    celPainter.span(painter, len);
    celPainter.advance(len);
    celPainter.border(painter);
  }
  
  painter.end();
  update();
}

void GroupsWidget::setGroup(const GroupInfo info) {
  selectionImg.fill(0);
  QPainter painter{&selectionImg};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  
  painter.fillRect(
    +info.begin * cel_width, 0,
    +(info.end - info.begin) * cel_width - glob_border_width, cel_height - glob_border_width,
    cel_pos_color
  );
  
  painter.end();
  update();
}

void GroupsWidget::setFrameCount(const FrameIdx count) {
  frames = count;
  resizeImage(groupImg, {+frames * cel_width, cel_height});
  resizeImage(selectionImg, {+frames * cel_width, cel_height});
  setWidth();
}

void GroupsWidget::setMargin(const int newMargin) {
  margin = newMargin;
  setWidth();
  update();
}

void GroupsWidget::setWidth() {
  setFixedWidth(+frames * cel_width + margin);
}

int GroupsWidget::clampPos(QMouseEvent *event) const {
  return std::clamp(event->pos().x(), 0, width() - 1);
}

FrameIdx GroupsWidget::framePos(const int pos) const {
  return FrameIdx{pos / cel_width};
}

std::optional<FrameIdx> GroupsWidget::boundaryPos(const int pos) const {
  const int relPos = pos % cel_width;
  const int frame = pos / cel_width;
  if (relPos < 1_px) {
    return FrameIdx{frame};
  } else if (relPos >= cel_width - glob_border_width - 1_px) {
    return FrameIdx{frame + 1};
  } else {
    return std::nullopt;
  }
}

void GroupsWidget::mousePressEvent(QMouseEvent *event) {
  const int pos = clampPos(event);
  if (auto end = boundaryPos(pos)) {
    if (auto group = findGroupBoundary(groupArray, *end)) {
      dragGroup = group;
      return;
    }
  }
  Q_EMIT shouldSetGroup(framePos(pos));
}

void GroupsWidget::mouseMoveEvent(QMouseEvent *event) {
  const int pos = clampPos(event);
  if (dragGroup) {
    if (auto end = boundaryPos(pos)) {
      Q_EMIT shouldMoveGroup(*dragGroup, *end);
    }
  } else {
    if (auto end = boundaryPos(pos)) {
      if (findGroupBoundary(groupArray, *end)) {
        setCursor(Qt::SplitHCursor);
        return;
      }
    }
    setCursor(Qt::ArrowCursor);
  }
}

void GroupsWidget::mouseReleaseEvent(QMouseEvent *) {
  dragGroup = std::nullopt;
}

void GroupsWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.drawImage(0, 0, selectionImg);
  painter.drawImage(0, 0, groupImg);
}

GroupScrollWidget::GroupScrollWidget(QWidget *parent)
  : QScrollArea{parent} {
  setFrameShape(NoFrame);
  setFixedHeight(cel_height);
  setStyleSheet("background-color:" + glob_main.name());
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

GroupsWidget *GroupScrollWidget::getChild() {
  auto *groups = new GroupsWidget{this};
  // We cannot simply call setViewportMargins
  CONNECT(this, shouldSetRightMargin, groups, setMargin);
  setWidget(groups);
  return groups;
}

void GroupScrollWidget::paintEvent(QPaintEvent *) {
  QPainter painter{viewport()};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.fillRect(
    0, height() - glob_border_width,
    width(), glob_border_width,
    glob_border_color
  );
}

#include "timeline cels widget.moc"
