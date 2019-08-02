//
//  sample object.hpp
//  Animera
//
//  Created by Indi Kernick on 3/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef sample_object_hpp
#define sample_object_hpp

#include <QtGui/qrgb.h>
#include <QtCore/qpoint.h>
#include <QtCore/qobject.h>

class Cell;

class SampleObject final : public QObject {
  Q_OBJECT

public Q_SLOTS:
  void setCell(const Cell *);
  void mouseMove(QPoint);
  void keyPress(Qt::Key);

Q_SIGNALS:
  void colorChanged(QRgb);
  
private:
  const Cell *cell = nullptr;
  QPoint pos;
};

#endif
