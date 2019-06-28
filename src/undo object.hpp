//
//  undo object.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef undo_object_hpp
#define undo_object_hpp

#include "undo.hpp"
#include <string_view>
#include <QtCore/qobject.h>

class Cell;

class UndoObject final : public QObject {
  Q_OBJECT

public Q_SLOTS:
  void changePos(Cell *);
  void keyPress(Qt::Key);
  void cellModified();

Q_SIGNALS:
  void cellReverted();
  void showTempStatus(std::string_view);

private:
  Cell *cell = nullptr;
  UndoStack stack;
  
  void undo();
  void redo();
};

#endif
