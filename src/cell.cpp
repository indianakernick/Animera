//
//  cell.cpp
//  Animera
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cell.hpp"

Cell::Cell(QImage image)
  : image{image} {
  image.detach();
}

Cell::Cell(const QSize size, const Format format)
  : image{size, qimageFormat(format)} {
  clearImage(image);
}

CellPtr Cell::clone() const {
  auto copy = std::make_unique<Cell>();
  copy->image = image;
  return copy;
}
