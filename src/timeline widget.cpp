//
//  timeline widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline widget.hpp"

TimelineWidget::TimelineWidget(QWidget *parent, Animation &anim)
  : QScrollArea{parent}, anim{anim} {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setContentsMargins(0, 0, 0, 0);
}

void TimelineWidget::projectLoaded() {
  visible.resize(anim.layerCount());
  std::fill(visible.begin(), visible.end(), true);
  Q_EMIT layerVisibility(visible);
  Q_EMIT posChange(anim.getCell(0, 0), 0, 0);
}

#include "timeline widget.moc"
