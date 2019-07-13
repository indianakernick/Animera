//
//  undo object.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "undo object.hpp"

#include "cell.hpp"
#include "config.hpp"

void UndoObject::setCell(Cell *newCell) {
  cell = newCell;
  if (cell) {
    stack.reset(cell->image);
  } else {
    stack.clear();
  }
}

void UndoObject::keyPress(const Qt::Key key) {
  if (key == key_undo) {
    undo();
  } else if (key == key_redo) {
    redo();
  }
}

void UndoObject::cellModified() {
  if (cell) {
    stack.modify(cell->image);
  }
}

void UndoObject::undo() {
  if (!cell) {
    Q_EMIT shouldShowTemp("Cannot undo actions on this cell");
    return;
  }
  UndoState state = stack.undo();
  if (state.undid) {
    cell->image = state.img;
    Q_EMIT cellReverted();
  } else {
    Q_EMIT shouldShowTemp("Cannot undo any further");
  }
}

void UndoObject::redo() {
  if (!cell) {
    Q_EMIT shouldShowTemp("Cannot redo actions on this cell");
    return;
  }
  UndoState state = stack.redo();
  if (state.undid) {
    cell->image = state.img;
    Q_EMIT cellReverted();
  } else {
    Q_EMIT shouldShowTemp("Cannot redo any further");
  }
}

#include "undo object.moc"
