//
//  clear object.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "clear object.hpp"

#include "config.hpp"
#include "cell impls.hpp"

void ClearObject::posChange(Cell *cell) {
  source = dynamic_cast<SourceCell *>(cell);
}

void ClearObject::keyPress(const Qt::Key key) {
  if (source && key == key_clear) {
    // @TODO clear with erase color
    clearImage(source->image.data);
    Q_EMIT cellModified();
  }
}

#include "clear object.moc"
