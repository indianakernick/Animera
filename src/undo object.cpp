//
//  undo object.cpp
//  Animera
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "undo object.hpp"

#include "config.hpp"
#include "composite.hpp"

void UndoObject::setCell(Cell *newCell) {
  if (cell != newCell) {
    stack.reset(newCell->image);
  }
  cell = newCell;
}

void UndoObject::keyPress(const Qt::Key key) {
  if (key == key_undo) {
    undo();
  } else if (key == key_redo) {
    redo();
  }
}

void UndoObject::cellModified() {
  stack.modify(cell->image);
}

void UndoObject::undo() {
  UndoState state = stack.undo();
  if (state.undid) {
    restore(state.img);
  } else {
    Q_EMIT shouldShowTemp("Cannot undo any further");
  }
}

void UndoObject::redo() {
  UndoState state = stack.redo();
  if (state.undid) {
    restore(state.img);
  } else {
    Q_EMIT shouldShowTemp("Cannot redo any further");
  }
}

void UndoObject::restore(const QImage &image) {
  if (cell->image.isNull() > image.isNull()) {
    Q_EMIT shouldGrowCell({image.offset(), image.size()});
    blitImage(cell->image, image, {});
  } else if (cell->image.isNull() < image.isNull()) {
    Q_EMIT shouldClearCell();
  } else {
    cell->image = image;
    Q_EMIT cellReverted();
  }
}

#include "undo object.moc"
