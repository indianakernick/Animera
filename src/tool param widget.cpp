//
//  tool param widget.cpp
//  Animera
//
//  Created by Indi Kernick on 8/12/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool param widget.hpp"

#include "config.hpp"

ToolParamWidget::ToolParamWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedHeight(stat_rect.widget().height());
}
