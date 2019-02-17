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
  QImage newImage(data.size(), data.format());
  newImage.fill(0);
  QPainter painter(&newImage);
  painter.translate(xform.posX, xform.posY);
  painter.translate(data.width() / 2, data.height() / 2);
  painter.scale(flipToScale(xform.flipX), flipToScale(xform.flipY));
  painter.rotate(angleToDegrees(xform.angle));
  painter.translate(data.width() / -2, data.height() / -2);
  painter.drawImage(0, 0, data);
  painter.end();
  return newImage;
}

bool Image::isNull() const {
  return data.isNull();
}
