//
//  undo object.hpp
//  Animera
//
//  Created by Indiana Kernick on 7/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_undo_object_hpp
#define animera_undo_object_hpp

#include "cel.hpp"
#include "undo.hpp"
#include <string_view>
#include <QtCore/qobject.h>

class UndoObject final : public QObject {
  Q_OBJECT

public:
  explicit UndoObject(QObject *);
  
public Q_SLOTS:
  void setCel(Cel *);
  void keyPress(Qt::Key);
  void celModified();

Q_SIGNALS:
  void celReverted(QRect);
  void shouldShowTemp(std::string_view);
  void shouldClearCel();
  void shouldGrowCel(QRect);

private:
  Cel *cel = nullptr;
  UndoStack stack;
  
  void undo();
  void redo();
  void restore(const Cel &);
};

#endif
