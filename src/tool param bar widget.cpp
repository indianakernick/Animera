//
//  tool param bar widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 8/12/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "tool param bar widget.hpp"

#include "config colors.hpp"
#include "config geometry.hpp"
#include "tool param widget.hpp"

ToolParamBarWidget::ToolParamBarWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedHeight(stat_rect.widget().height());
  setStyleSheet("background-color:" + glob_main.name());
}
