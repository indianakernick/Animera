//
//  undo object.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "undo object.hpp"

#include "config.hpp"
#include "cell impls.hpp"

void UndoObject::posChange(Cell *cell) {
  // @TODO support transform cells
  if ((source = dynamic_cast<SourceCell *>(cell))) {
    stack.reset(source->image);
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
  if (source) {
    stack.modify(source->image);
  }
}

void UndoObject::undo() {
  if (!source) {
    Q_EMIT showTempStatus("Cannot undo actions on this cell");
    return;
  }
  UndoState state = stack.undo();
  if (state.undid) {
    source->image = state.img;
    Q_EMIT cellReverted();
  } else {
    Q_EMIT showTempStatus("Cannot undo any further");
  }
}

void UndoObject::redo() {
  if (!source) {
    Q_EMIT showTempStatus("Cannot redo actions on this cell");
    return;
  }
  UndoState state = stack.redo();
  if (state.undid) {
    source->image = state.img;
    Q_EMIT cellReverted();
  } else {
    Q_EMIT showTempStatus("Cannot redo any further");
  }
}

#include "undo object.moc"
