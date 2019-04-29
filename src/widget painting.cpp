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

void paintBorder(QPainter &painter, const RectWidgetSize size, const QColor color) {
  // @TODO this is lazy and inefficient
  // would be more efficient to render 4 rects
  painter.setPen(Qt::NoPen);
  painter.setBrush(color);
  painter.setClipRegion(QRegion{size.outer()}.subtracted(size.inner()));
  painter.drawRect(size.outer());
  painter.setClipRect(size.widget());
}
