//
//  cell.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cell.hpp"

Cell::Cell(Image image)
  : image{image} {
  image.data.detach();
}

Cell::Cell(const QSize size, const Format format, Palette *palette)
  : image{{size, qimageFormat(format)}, palette} {
  clearImage(image.data);
}

CellPtr Cell::clone() const {
  auto copy = std::make_unique<Cell>();
  copy->image = image;
  return copy;
}
