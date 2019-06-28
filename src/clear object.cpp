//
//  clear object.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "clear object.hpp"

#include "cell.hpp"
#include "config.hpp"

void ClearObject::changePos(Cell *newCell) {
  cell = newCell;
}

void ClearObject::keyPress(const Qt::Key key) {
  if (cell && key == key_clear) {
    clearImage(cell->image.data, color);
    Q_EMIT cellModified();
  }
}

void ClearObject::changeColors(const ToolColors colors) {
  color = colors.erase;
}

#include "clear object.moc"
