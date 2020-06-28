//
//  widget painting.hpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_widget_painting_hpp
#define animera_widget_painting_hpp

#include "widget rect.hpp"
#include <QtGui/qpixmap.h>

QPixmap bakeColoredBitmap(const QString &, QColor);
QPixmap bakeColoredBitmap(const QBitmap &, QColor);
QPixmap bakeColoredBitmaps(const QString &, const QString &, QColor, QColor);
QPixmap bakeColoredBitmaps(const QBitmap &, const QBitmap &, QColor, QColor);
void paintBorder(QPainter &, const WidgetRect &, QColor);
void paintChecker(QPainter &, const WidgetRect &, int);

#endif
