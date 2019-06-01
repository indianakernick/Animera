//
//  widget painting.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "widget painting.hpp"

#include "config.hpp"
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>

QPixmap bakeColoredBitmaps(
  const QString &pathP,
  const QString &pathS,
  const QColor colorP,
  const QColor colorS
) {
  QBitmap bitmapP{pathP};
  QBitmap bitmapS{pathS};
  assert(bitmapP.size() == bitmapS.size());
  bitmapP = bitmapP.scaled(bitmapP.size() * glob_scale);
  bitmapS = bitmapS.scaled(bitmapS.size() * glob_scale);
  return bakeColoredBitmaps(bitmapP, bitmapS, colorP, colorS);
}

QPixmap bakeColoredBitmaps(
  const QBitmap &bitmapP,
  const QBitmap &bitmapS,
  const QColor colorP,
  const QColor colorS
) {
  assert(bitmapP);
  assert(bitmapS);
  assert(bitmapP.size() == bitmapS.size());
  QPixmap pixmap{bitmapP.size()};
  pixmap.fill(QColor{0, 0, 0, 0});
  const QRect rect = {QPoint{}, pixmap.size()};
  QPainter painter{&pixmap};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setClipRegion(bitmapP);
  painter.fillRect(rect, colorP);
  painter.setClipRegion(bitmapS);
  painter.fillRect(rect, colorS);
  return pixmap;
}

void paintBorder(QPainter &painter, const WidgetRect rect, const QColor color) {
  const QSize innerSize = rect.inner().size();
  const QRect rects[4] = {
    { // top
      rect.widgetSpace(), rect.widgetSpace(),
      innerSize.width() + 2 * rect.borderWidth(), rect.borderWidth()
    },
    { // bottom
      rect.widgetSpace(), rect.widgetSpace() + rect.borderWidth() + innerSize.height(),
      innerSize.width() + 2 * rect.borderWidth(), rect.borderWidth()
    },
    { // left
      rect.widgetSpace(), rect.widgetSpace() + rect.borderWidth(),
      rect.borderWidth(), innerSize.height()
    },
    { // right
      rect.widgetSpace() + rect.borderWidth() + innerSize.width(), rect.widgetSpace() + rect.borderWidth(),
      rect.borderWidth(), innerSize.height()
    }
  };
  painter.setPen(Qt::NoPen);
  painter.setBrush(color);
  painter.drawRects(rects, 4);
}

void paintChecker(
  QPainter &painter,
  const WidgetRect rect,
  const int vertTiles
) {
  const QRect inner = rect.inner();
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor{edit_checker_a});
  painter.drawRect(inner);
  painter.setBrush(QColor{edit_checker_b});
  painter.setClipRect(inner);
  const int tileSize = inner.height() / vertTiles;
  const int horiTiles = inner.width() / tileSize;
  for (int y = 0; y != vertTiles; ++y) {
    for (int x = 1 - y; x <= horiTiles; x += 2) {
      painter.drawRect(
        inner.x() + tileSize * x,
        inner.y() + tileSize * y,
        tileSize,
        tileSize
      );
    }
  }
  painter.setClipRect(rect.widget());
}
