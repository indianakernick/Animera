//
//  undo object.hpp
//  Animera
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef undo_object_hpp
#define undo_object_hpp

#include "undo.hpp"
#include "cell.hpp"
#include <string_view>
#include <QtCore/qobject.h>

class UndoObject final : public QObject {
  Q_OBJECT

public:
  explicit UndoObject(QObject *);
  
public Q_SLOTS:
  void setCell(Cell *);
  void keyPress(Qt::Key);
  void cellModified();

Q_SIGNALS:
  void cellReverted(QRect);
  void shouldShowTemp(std::string_view);
  void shouldClearCell();
  void shouldGrowCell(QRect);

private:
  Cell *cell = nullptr;
  UndoStack stack;
  
  void undo();
  void redo();
  void restore(const Cell &);
};

#endif
