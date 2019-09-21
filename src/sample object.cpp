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
  if (key == key_sample) {
    // TODO: should we sample from composited image?
    if (cell->rect().contains(pos)) {
      Q_EMIT colorChanged(cell->image.pixel(pos - cell->pos()));
    } else {
      Q_EMIT colorChanged(0);
    }
  }
}

#include "sample object.moc"
