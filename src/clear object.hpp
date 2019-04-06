//
//  clear object.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef clear_object_hpp
#define clear_object_hpp

#include <QtCore/qobject.h>

class Cell;
class SourceCell;

// @TODO is this class really the right way to go about the problem?
class ClearObject final : public QObject {
  Q_OBJECT
  
public Q_SLOTS:
  void posChange(Cell *);
  void keyPress(Qt::Key);
  
Q_SIGNALS:
  void cellModified();

private:
  SourceCell *source = nullptr;
};

#endif
