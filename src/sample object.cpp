//
//  sample object.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "sample object.hpp"

#include "cell.hpp"
#include "config.hpp"

void SampleObject::setCell(Cell *newCell) {
  cell = newCell;
}

void SampleObject::mouseMove(const QPoint newPos) {
  pos = newPos;
}

void SampleObject::keyPress(const Qt::Key key) {
  if (cell && key == key_sample) {
    Q_EMIT colorChanged(cell->image.pixel(pos));
  }
}

#include "sample object.moc"
