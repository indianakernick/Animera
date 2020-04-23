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

void UndoObject::setCel(Cel *newCel) {
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

void UndoObject::celModified() {
  SCOPE_TIME("UndoObject::celModified");
  
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

void UndoObject::restore(const Cel &newCel) {
  SCOPE_TIME("UndoObject::restore");
  
  if (cel->isNull() > newCel.isNull()) {
    Q_EMIT shouldGrowCel(newCel.rect());
    copyImage(cel->img, newCel.img);
    Q_EMIT celReverted(newCel.rect());
  } else if (cel->isNull() < newCel.isNull()) {
    Q_EMIT shouldClearCel();
  } else {
    const QRect rect = cel->rect().united(newCel.rect());
    *cel = newCel;
    Q_EMIT celReverted(rect);
  }
}

#include "undo object.moc"
