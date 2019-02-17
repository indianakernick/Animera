//
//  image.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef pixel_2_image_hpp
#define pixel_2_image_hpp

#include <QtGui/qimage.h>

struct Transform {
  QPoint pos = {0, 0};
  QPoint scale = {1, 1};
  int rot = 0;
};

struct Image {
  QImage data;
  Transform xform;
  
  QImage transformed() const;
};

#endif
