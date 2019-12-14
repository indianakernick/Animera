//
//  status object.hpp
//  Animera
//
//  Created by Indi Kernick on 12/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef status_object_hpp
#define status_object_hpp

#include "cell.hpp"
#include <QtCore/qobject.h>

class StatusObject final : public QObject {
  Q_OBJECT

public:
  explicit StatusObject(QObject *);

public Q_SLOTS:
  void setScale(int);
  void setCurrPos(CellPos);
  void setSelection(CellRect);
  void setFrameCount(FrameIdx);
  void setLayerCount(LayerIdx);

Q_SIGNALS:
  void shouldShowPerm(std::string_view);
  void shouldShowApnd(std::string_view);

private:
  int scale;
  CellPos currPos;
  CellRect selection;
  FrameIdx frameCount;
  LayerIdx layerCount;
  
  void updateStatus();
};

#endif
