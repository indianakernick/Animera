//
//  undo object.cpp
//  Animera
//
//  Created by Indiana Kernick on 7/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "undo object.hpp"

#include "scope time.hpp"
#include "config keys.hpp"

UndoObject::UndoObject(QObject *parent)
  : QObject{parent} {}

void UndoObject::setCelImage(CelImage *newCel) {
  if (cel != newCel) {
    stack.reset(*newCel);
  }
  cel = newCel;
}

void UndoObject::keyPress(const Qt::Key key) {
  if (key == key_undo) {
    undo();
  } else if (key == key_redo) {
    redo();
  }
}

void UndoObject::celImageModified() {
  SCOPE_TIME("UndoObject::celImageModified");
  
  // TODO: not notified of cels being cleared or pasted onto
  // maybe we could listen to the timeline.modified signal?
  stack.modify(*cel);
}

void UndoObject::undo() {
  UndoState state = stack.undo();
  if (state.undid) {
    restore(state.cel);
  } else {
    Q_EMIT shouldShowTemp("Cannot undo any further");
  }
}

void UndoObject::redo() {
  UndoState state = stack.redo();
  if (state.undid) {
    restore(state.cel);
  } else {
    Q_EMIT shouldShowTemp("Cannot redo any further");
  }
}

void UndoObject::restore(const CelImage &newCel) {
  SCOPE_TIME("UndoObject::restore");
  
  if (cel->isNull() > newCel.isNull()) {
    Q_EMIT shouldGrowCelImage(newCel.rect());
    copyImage(cel->img, newCel.img);
    Q_EMIT celImageReverted(newCel.rect());
  } else if (cel->isNull() < newCel.isNull()) {
    Q_EMIT shouldClearCel();
  } else {
    const QRect rect = cel->rect().united(newCel.rect());
    *cel = newCel;
    Q_EMIT celImageReverted(rect);
  }
}

#include "undo object.moc"
