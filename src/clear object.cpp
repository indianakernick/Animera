//
//  clear object.cpp
//  Animera
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "clear object.hpp"

#include "cell.hpp"
#include "config.hpp"

void ClearObject::setCell(Cell *newCell) {
  cell = newCell;
}

void ClearObject::keyPress(const Qt::Key key) {
  if (cell && key == key_clear) {
    clearImage(cell->image, color);
    Q_EMIT cellModified();
  }
}

void ClearObject::setColors(const ToolColors colors) {
  color = colors.erase;
}

#include "clear object.moc"
