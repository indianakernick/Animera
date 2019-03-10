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
  setMinimumHeight(128);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
}

#include "timeline widget.moc"
