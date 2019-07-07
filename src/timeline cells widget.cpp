//
//  timeline cells widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline cells widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qpainter.h>
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

CellsWidget::CellsWidget(QWidget *parent)
: QWidget{parent},
  currPosImg{0, 0, QImage::Format_ARGB32_Premultiplied},
  layersImg{0, 0, QImage::Format_ARGB32_Premultiplied} {
  cellPix = bakeColoredBitmap(":/Timeline/cell.pbm", cell_icon_color);
  beginLinkPix = bakeColoredBitmap(":/Timeline/begin linked cell.pbm", cell_icon_color);
  endLinkPix = bakeColoredBitmap(":/Timeline/end linked cell.pbm", cell_icon_color);
}

void CellsWidget::setCurrPos(const CellPos pos) {
  const QPoint pixelPos = {pos.f * cell_width, pos.l * cell_height};
  Q_EMIT ensureVisible(pixelPos);
  currPosImg.fill(0);
  QPainter painter{&currPosImg};
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor{0, 0, 0, 0});
  painter.drawRect(currPosImg.rect());
  painter.setBrush(cell_curr_color);
  constexpr int size = 2 * cell_icon_pad + cell_icon_size;
  if (height() > cell_height || width() == cell_width) {
    painter.drawRect(0, pos.l * cell_height, width(), size);
  }
  if (width() > cell_width) {
    painter.drawRect(pos.f * cell_width, 0, size, height());
  }
  painter.end();
  repaint();
  
  /*
  if (select.minL <= select.maxL && select.minF <= select.maxF) {
    const int border2 = 2 * glob_border_width;
    const int width = (select.maxF - select.minF) * cell_width + size;
    const int height = (select.maxL - select.minL) * cell_width + size;
    painter.setBrush(cell_select_color);
    painter.drawRect( // top
      select.minF * cell_width, select.minL * cell_height,
      width, glob_border_width
    );
    painter.drawRect( // bottom
      select.minF * cell_width, select.maxL * cell_height + size - glob_border_width,
      width, glob_border_width
    );
    painter.drawRect( // left
      select.minF * cell_width, select.minL * cell_height + glob_border_width,
      glob_border_width, height - border2
    );
    painter.drawRect( // right
      select.maxF * cell_width + size - glob_border_width, select.minL * cell_height + glob_border_width,
      glob_border_width, height - border2
    );
  }
  */
}

namespace {

void paintBorder(QPainter &painter, const int x, const int y) {
  painter.fillRect(
    x - cell_border_offset, y,
    glob_border_width, cell_height,
    glob_border_color
  );
}

}

void CellsWidget::setLayer(const LayerIdx idx, const Spans &spans) {
  QPainter painter{&layersImg};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.fillRect(0, idx * cell_height, width(), cell_height, QColor{0, 0, 0, 0});
  int x = cell_icon_pad;
  const int y = idx * cell_height;
  for (const CellSpan &span : spans) {
    if (span.cell) {
      if (span.len == 1) {
        painter.drawPixmap(x, y + cell_icon_pad, cellPix);
        x += cell_width;
        paintBorder(painter, x, y);
      } else if (span.len > 1) {
        const int between = (span.len - 2) * cell_width;
        painter.drawPixmap(x, y + cell_icon_pad, beginLinkPix);
        x += cell_width;
        painter.fillRect(
          x - cell_icon_pad - cell_border_offset, y + cell_icon_pad,
          between + cell_icon_pad + cell_border_offset, cell_icon_size,
          cell_icon_color
        );
        x += between;
        painter.drawPixmap(x, y + cell_icon_pad, endLinkPix);
        x += cell_width;
        paintBorder(painter, x, y);
      } else Q_UNREACHABLE();
    } else {
      for (FrameIdx f = 0; f != span.len; ++f) {
        x += cell_width;
        paintBorder(painter, x, y);
      }
    }
  }
  painter.fillRect(
    0, y + cell_height - glob_border_width,
    x - cell_icon_pad, glob_border_width,
    glob_border_color
  );
  painter.end();
  repaint();
}

void CellsWidget::setFrameCount(const FrameIdx count) {
  setFixedWidth(count * cell_width);
  resizeImage(currPosImg, size());
  resizeImage(layersImg, size());
  Q_EMIT resized();
}

void CellsWidget::setLayerCount(const LayerIdx count) {
  setFixedHeight(count * cell_height);
  resizeImage(currPosImg, size());
  resizeCopyImage(layersImg, size());
  Q_EMIT resized();
}

void CellsWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.drawImage(0, 0, currPosImg);
  painter.drawImage(0, 0, layersImg);
}

void CellsWidget::focusOutEvent(QFocusEvent *) {
  //select = {-1, -1, -1, -1};
}

CellScrollWidget::CellScrollWidget(QWidget *parent)
  : ScrollAreaWidget{parent} {
  setFrameShape(NoFrame);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setStyleSheet("background-color:" + glob_main.name());
  setMinimumSize(cell_width + glob_scroll_width, cell_height + glob_scroll_width);
}

CellsWidget *CellScrollWidget::getChild() {
  auto *cells = new CellsWidget{this};
  rect = new QWidget{cells};
  rect->setVisible(false);
  CONNECT(cells, ensureVisible, this, ensureVisible);
  CONNECT(cells, resized, this, contentResized);
  setWidget(cells);
  return cells;
}

void CellScrollWidget::contentResized() {
  const QMargins before = viewportMargins();
  adjustMargins();
  const QMargins after = viewportMargins();
  if (before.right() != after.right()) {
    Q_EMIT rightMarginChanged(after.right());
  }
  if (before.bottom() != after.bottom()) {
    Q_EMIT bottomMarginChanged(after.bottom());
  }
}

void CellScrollWidget::ensureVisible(const QPoint pos) {
  // @TODO Why do I have to add 1 here? Bug?
  rect->setGeometry(pos.x(), pos.y(), cell_width + 1, cell_height + 1);
  QScrollArea::ensureWidgetVisible(rect, 0, 0);
}

void CellScrollWidget::resizeEvent(QResizeEvent *event) {
  contentResized();
  ScrollAreaWidget::resizeEvent(event);
}

#include "timeline cells widget.moc"
