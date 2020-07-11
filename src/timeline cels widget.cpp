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
#include <QtGui/qpainter.h>
#include "config colors.hpp"
#include "config geometry.hpp"
#include "widget painting.hpp"

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
  celPix = bakeColoredBitmap(":/Timeline/cel.png", cel_icon_color);
  beginLinkPix = bakeColoredBitmap(":/Timeline/begin linked cel.png", cel_icon_color);
  endLinkPix = bakeColoredBitmap(":/Timeline/end linked cel.png", cel_icon_color);
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

namespace {

void paintBorder(QPainter &painter, const int x, const int y) {
  painter.fillRect(
    x - cel_border_offset, y,
    glob_border_width, cel_height,
    glob_border_color
  );
}

template <typename Func>
void apply(QPainter &a, QPainter &b, Func func) {
  func(a);
  func(b);
}

}

void CelsWidget::setLayer(const LayerIdx idx, tcb::span<const CelSpan> spans) {
  QPainter layers{&layersImg};
  layers.setCompositionMode(QPainter::CompositionMode_Source);
  QPainter borders{&bordersImg};
  borders.setCompositionMode(QPainter::CompositionMode_Source);
  
  apply(layers, borders, [this, idx](QPainter &painter) {
    painter.fillRect(0, +idx * cel_height, width(), cel_height, QColor{0, 0, 0, 0});
  });
  
  int x = cel_icon_pad;
  const int y = +idx * cel_height;
  for (const CelSpan &span : spans) {
    if (*span.cel) {
      if (span.len == FrameIdx{1}) {
        apply(layers, borders, [this, x, y](QPainter &painter) {
          painter.drawPixmap(x, y + cel_icon_pad, celPix);
        });
        x += cel_width;
        paintBorder(borders, x, y);
      } else if (span.len > FrameIdx{1}) {
        const int between = +(span.len - FrameIdx{2}) * cel_width;
        apply(layers, borders, [this, x, y](QPainter &painter) {
          painter.drawPixmap(x, y + cel_icon_pad, beginLinkPix);
        });
        x += cel_width;
        apply(layers, borders, [between, x, y](QPainter &painter) {
          painter.fillRect(
            x - cel_icon_pad - cel_border_offset, y + cel_icon_pad,
            between + cel_icon_pad + cel_border_offset, cel_icon_size,
            cel_icon_color
          );
        });
        x += between;
        apply(layers, borders, [this, x, y](QPainter &painter) {
          painter.drawPixmap(x, y + cel_icon_pad, endLinkPix);
        });
        x += cel_width;
        paintBorder(borders, x, y);
      } else Q_UNREACHABLE();
    } else {
      for (FrameIdx f = {}; f != span.len; ++f) {
        x += cel_width;
        paintBorder(borders, x, y);
      }
    }
  }
  
  borders.fillRect(
    0, y + cel_height - glob_border_width,
    x - cel_icon_pad, glob_border_width,
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
    groupImg{0, 0, QImage::Format_ARGB32_Premultiplied} {
  celPix = bakeColoredBitmap(":/Timeline/cel.png", cel_icon_color);
  beginLinkPix = bakeColoredBitmap(":/Timeline/begin linked cel.png", cel_icon_color);
  endLinkPix = bakeColoredBitmap(":/Timeline/end linked cel.png", cel_icon_color);
  setFixedSize(0, cel_height);
}

// TODO: Move cell span painting into a single class
// Also try to share the resources (pixmaps)

void GroupsWidget::setGroups(const tcb::span<const Group> groups) {
  groupImg.fill(0);
  
  QPainter painter{&groupImg};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  
  int x = cel_icon_pad;
  FrameIdx prevEnd = {};
  for (const Group &group : groups) {
    const FrameIdx len = group.end - prevEnd;
    prevEnd = group.end;
    if (len == FrameIdx{1}) {
      painter.drawPixmap(x, cel_icon_pad, celPix);
      x += cel_width;
      paintBorder(painter, x, 0);
    } else if (len > FrameIdx{1}) {
      const int between = +(len - FrameIdx{2}) * cel_width;
      painter.drawPixmap(x, cel_icon_pad, beginLinkPix);
      x += cel_width;
      painter.fillRect(
        x - cel_icon_pad - cel_border_offset, cel_icon_pad,
        between + cel_icon_pad + cel_border_offset, cel_icon_size,
        cel_icon_color
      );
      x += between;
      painter.drawPixmap(x, cel_icon_pad, endLinkPix);
      x += cel_width;
      paintBorder(painter, x, 0);
    } else Q_UNREACHABLE();
  }
  
  painter.end();
  update();
}

void GroupsWidget::setFrameCount(const FrameIdx count) {
  frames = count;
  resizeImage(groupImg, {+frames * cel_width, cel_height});
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

void GroupsWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
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
