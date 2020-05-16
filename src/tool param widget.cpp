//
//  tool param widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 8/12/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "tool param widget.hpp"

#include "config.hpp"

ToolParamWidget::ToolParamWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedHeight(stat_rect.widget().height());
}
