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

QPixmap bakeColoredBitmap(const QString &path, const QColor color) {
  QBitmap bitmap{path};
  return bakeColoredBitmap(bitmap.scaled(bitmap.size() * glob_scale), color);
}

QPixmap bakeColoredBitmap(const QBitmap &bitmap, const QColor color) {
  assert(bitmap);
  QPixmap pixmap{bitmap.size()};
  pixmap.fill(QColor{0, 0, 0, 0});
  QPainter painter{&pixmap};
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setClipRegion(bitmap);
  painter.fillRect(pixmap.rect(), color);
  return pixmap;
}

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
  /*const QSize innerSize = rect.inner().size();
  const int widgetSpace = rect.outer().x() - rect.widget().x();
  const int borderWidth = rect.inner().x() - rect.outer().x();
  const QRect rects[4] = {
    { // top
      widgetSpace, widgetSpace,
      innerSize.width() + 2 * borderWidth, borderWidth
    },
    { // bottom
      widgetSpace, widgetSpace + borderWidth + innerSize.height(),
      innerSize.width() + 2 * borderWidth, borderWidth
    },
    { // left
      widgetSpace, widgetSpace + borderWidth,
      borderWidth, innerSize.height()
    },
    { // right
      widgetSpace + borderWidth + innerSize.width(), widgetSpace + borderWidth,
      borderWidth, innerSize.height()
    }
  };
  painter.setPen(Qt::NoPen);
  painter.setBrush(color);
  painter.drawRects(rects, 4);*/
  painter.save();
  painter.setClipRegion(QRegion{rect.outer()} - QRegion{rect.inner()});
  painter.fillRect(rect.outer(), color);
  painter.restore();
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
