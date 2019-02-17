//
//  image.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef image_hpp
#define image_hpp

#include <QtGui/qimage.h>

struct Transform {
  qint16 posX = 0;
  qint16 posY = 0;
  quint8 angle = 0;
  bool flipX = false;
  bool flipY = false;
};

inline quint8 increaseAngle(const quint8 angle) {
  return (angle + 1) & 3;
}

inline quint8 decreaseAngle(const quint8 angle) {
  return (angle + 3) & 3;
}

inline qreal angleToDegrees(const quint8 angle) {
  return angle * 90.0;
}

inline qreal flipToScale(const bool flip) {
  return static_cast<qreal>(flip) * -2.0 + 1.0;
}

struct Image {
  QImage data;
  Transform xform;
  
  QImage transformed() const;
  bool isNull() const;
};

#endif
