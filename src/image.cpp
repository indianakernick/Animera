//
//  image.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "image.hpp"

#include <QtGui/qpainter.h>

QImage Image::transformed() const {
  const QSize newSize{
    data.width() * qAbs(xform.scale.x()),
    data.height() * qAbs(xform.scale.y())
  };
  QImage newImage(newSize, data.format());
  newImage.fill(0);
  QPainter painter(&newImage);
  painter.translate(xform.pos);
  painter.translate(newSize.width() / 2, newSize.height() / 2);
  painter.scale(xform.scale.x(), xform.scale.y());
  painter.rotate(90.0 * xform.rot);
  painter.translate(data.width() / -2, data.height() / -2);
  painter.drawImage(0, 0, data);
  painter.end();
  return newImage;
}
