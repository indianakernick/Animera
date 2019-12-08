//
//  tool param bar widget.cpp
//  Animera
//
//  Created by Indi Kernick on 8/12/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool param bar widget.hpp"

#include "config.hpp"
#include "tool param widget.hpp"

ToolParamBarWidget::ToolParamBarWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedHeight(stat_rect.widget().height());
  setStyleSheet("background-color:" + glob_main.name());
}
