//
//  cell.cpp
//  Animera
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cell.hpp"

// @TODO remove this hack
Cell::Cell(const Format format)
  : image{1, 1, qimageFormat(format)} {
  image.setPixel(0, 0, 0);
}
