//
//  undo object.cpp
//  Animera
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "undo object.hpp"

#include "config.hpp"

UndoObject::UndoObject(QObject *parent)
  : QObject{parent} {}

void UndoObject::setCell(Cell *newCell) {
  if (cell != newCell) {
    stack.reset(*newCell);
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
  // TODO: not notified of cells being cleared or pasted onto
  // maybe we could listen to the timeline.modified signal?
  stack.modify(*cell);
}

void UndoObject::undo() {
  UndoState state = stack.undo();
  if (state.undid) {
    restore(state.cell);
  } else {
    Q_EMIT shouldShowTemp("Cannot undo any further");
  }
}

void UndoObject::redo() {
  UndoState state = stack.redo();
  if (state.undid) {
    restore(state.cell);
  } else {
    Q_EMIT shouldShowTemp("Cannot redo any further");
  }
}

void UndoObject::restore(const Cell &newCell) {
  if (cell->isNull() > newCell.isNull()) {
    Q_EMIT shouldGrowCell(newCell.rect());
    copyImage(cell->img, newCell.img);
    Q_EMIT cellReverted();
  } else if (cell->isNull() < newCell.isNull()) {
    Q_EMIT shouldClearCell();
  } else {
    *cell = newCell;
    Q_EMIT cellReverted();
  }
}

#include "undo object.moc"
