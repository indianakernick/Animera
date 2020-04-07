//
//  sample object.hpp
//  Animera
//
//  Created by Indiana Kernick on 3/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef sample_object_hpp
#define sample_object_hpp

#include "image.hpp"
#include <QtGui/qrgb.h>
#include <QtCore/qpoint.h>
#include <QtCore/qobject.h>

class Cell;

class SampleObject final : public QObject {
  Q_OBJECT

public:
  explicit SampleObject(QObject *);
  
public Q_SLOTS:
  void initCanvas(Format);
  void setCell(const Cell *);
  void mouseMove(QPoint);
  void keyPress(Qt::Key);

Q_SIGNALS:
  void shouldSetColor(QRgb);
  void shouldSetIndex(int);
  
private:
  const Cell *cell = nullptr;
  QPoint pos;
  Format format;
};

#endif
