//
//  sample object.cpp
//  Animera
//
//  Created by Indi Kernick on 3/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "sample object.hpp"

#include "cell.hpp"
#include "config.hpp"

void SampleObject::setCell(const Cell *newCell) {
  cell = newCell;
}

void SampleObject::mouseMove(const QPoint newPos) {
  pos = newPos;
}

void SampleObject::keyPress(const Qt::Key key) {
  if (cell && key == key_sample) {
    // @TODO should we sample from composited image?
    if (QRect{cell->image.offset(), cell->image.size()}.contains(pos)) {
      Q_EMIT colorChanged(cell->image.pixel(pos - cell->image.offset()));
    } else {
      Q_EMIT colorChanged(0);
    }
  }
}

#include "sample object.moc"
