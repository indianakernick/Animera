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
#include "composite.hpp"

void SampleObject::initCanvas(const Format newFormat) {
  format = newFormat;
}

void SampleObject::setCell(const Cell *newCell) {
  cell = newCell;
}

void SampleObject::mouseMove(const QPoint newPos) {
  pos = newPos;
}

void SampleObject::keyPress(const Qt::Key key) {
  if (key == key_sample) {
    const QRgb pixel = sampleCell(*cell, pos);
    // similar to right-clicking on a palette color
    if (format == Format::index) {
      Q_EMIT shouldSetIndex(pixel);
    } else {
      Q_EMIT shouldSetColor(pixel);
    }
  }
}

#include "sample object.moc"
