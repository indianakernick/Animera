//
//  sample object.hpp
//  Animera
//
//  Created by Indiana Kernick on 3/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_sample_object_hpp
#define animera_sample_object_hpp

#include "image.hpp"
#include <QtCore/qpoint.h>
#include <QtCore/qobject.h>

class Cel;

class SampleObject final : public QObject {
  Q_OBJECT

public:
  explicit SampleObject(QObject *);
  
public Q_SLOTS:
  void initCanvas(Format);
  void setCel(const Cel *);
  void mouseMove(QPoint);
  void keyPress(Qt::Key);

Q_SIGNALS:
  void shouldSetColor(PixelVar);
  void shouldSetIndex(PixelIndex);
  
private:
  const Cel *cel = nullptr;
  QPoint pos;
  Format format;
};

#endif
