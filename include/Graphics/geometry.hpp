//
//  geometry.hpp
//  Animera
//
//  Created by Indi Kernick on 1/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_geometry_hpp
#define graphics_geometry_hpp

namespace gfx {

struct Point {
  int x, y;
};

struct Size {
  int w, h;
};

struct Rect {
  Point p;
  Size s;
};

}

#endif
