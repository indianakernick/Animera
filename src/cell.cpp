//
//  cell.cpp
//  Animera
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cell.hpp"

Cell::Cell(const QSize size, const Format format)
  : image{size, qimageFormat(format)} {
  clearImage(image);
}
