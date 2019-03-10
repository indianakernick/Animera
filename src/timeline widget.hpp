//
//  timeline widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_widget_hpp
#define timeline_widget_hpp

#include "animation.hpp"
#include <QtWidgets/qscrollarea.h>

class TimelineWidget final : public QScrollArea {
  Q_OBJECT

public:
  TimelineWidget(QWidget *, Animation &);
  
Q_SIGNALS:
  // emitted when the current layer/frame has changed
  // or the current cell has changed
  void posChange(Cell *, LayerIdx, FrameIdx);
  // emitted when any layers are shown or hidden
  void layerVisibility(const LayerVisible &);

private:
  Animation &anim;
};

#endif
