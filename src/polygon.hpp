//
//  polygon.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 26/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef polygon_hpp
#define polygon_hpp

#include <vector>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>

class Polygon {
public:
  void init(QPoint);
  void push(QPoint);
  
  QRect bounds() const {
    return rect;
  }
  
  size_t size() const {
    return points.size();
  }
  auto begin() const {
    return points.begin();
  }
  auto end() const {
    return points.end();
  }
  
private:
  std::vector<QPoint> points;
  QRect rect;
};

#endif
