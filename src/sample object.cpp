//
//  sample object.cpp
//  Animera
//
//  Created by Indiana Kernick on 3/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "sample object.hpp"

#include "cel.hpp"
#include "composite.hpp"
#include "config keys.hpp"

SampleObject::SampleObject(QObject *parent)
  : QObject{parent} {}

void SampleObject::initCanvas(const Format newFormat) {
  format = newFormat;
}

void SampleObject::setCelImage(const CelImage *newCel) {
  cel = newCel;
}

void SampleObject::mouseMove(const QPoint newPos) {
  pos = newPos;
}

void SampleObject::keyPress(const Qt::Key key) {
  if (key == key_sample) {
    const PixelVar pixel = sampleCelImage(*cel, pos);
    // similar to right-clicking on a palette color
    if (format == Format::index) {
      Q_EMIT shouldSetIndex(static_cast<PixelIndex>(pixel));
    } else {
      Q_EMIT shouldSetColor(pixel);
    }
  }
}

#include "sample object.moc"
