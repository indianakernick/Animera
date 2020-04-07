//
//  undo object.cpp
//  Animera
//
//  Created by Indiana Kernick on 7/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "undo object.hpp"

#include "config.hpp"
#include "scope time.hpp"

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
  SCOPE_TIME("UndoObject::cellModified");
  
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
  SCOPE_TIME("UndoObject::restore");
  
  if (cell->isNull() > newCell.isNull()) {
    Q_EMIT shouldGrowCell(newCell.rect());
    copyImage(cell->img, newCell.img);
    Q_EMIT cellReverted(newCell.rect());
  } else if (cell->isNull() < newCell.isNull()) {
    Q_EMIT shouldClearCell();
  } else {
    const QRect rect = cell->rect().united(newCell.rect());
    *cell = newCell;
    Q_EMIT cellReverted(rect);
  }
}

#include "undo object.moc"
