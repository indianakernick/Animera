//
//  clear object.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef clear_object_hpp
#define clear_object_hpp

#include "tool.hpp"
#include <QtCore/qobject.h>

class Cell;

// @TODO is this class really the right way to go about the problem?
class ClearObject final : public QObject {
  Q_OBJECT
  
public Q_SLOTS:
  void posChange(Cell *);
  void keyPress(Qt::Key);
  void changeColors(ToolColors);
  
Q_SIGNALS:
  void cellModified();

private:
  Cell *cell = nullptr;
  QRgb color = qRgba(0, 0, 0, 0);
};

#endif
