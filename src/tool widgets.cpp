//
//  tool widgets.cpp
//  Animera
//
//  Created by Indi Kernick on 24/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool widgets.hpp"

#include "connect.hpp"
#include "label widget.hpp"
#include "combo box widget.hpp"
#include "color input widget.hpp"
#include <QtWidgets/qboxlayout.h>

namespace {

SymmetryMode symmetryFromStr(const QString &str) {
         if (str == "None") {
    return SymmetryMode::none;
  } else if (str == "Horizontal") {
    return SymmetryMode::hori;
  } else if (str == "Vertical") {
    return SymmetryMode::vert;
  } else if (str == "Both") {
    return SymmetryMode::both;
  } else Q_UNREACHABLE();
}

}

void BrushToolWidget::createWidgets() {
  radius = new NumberInputWidget{this, pick_number_rect, brsh_radius};
  symmetry = new ComboBoxWidget{this, 10};
  symmetry->clearWithItem("None");
  symmetry->addItem("Horizontal");
  symmetry->addItem("Vertical");
  symmetry->addItem("Both");
  symmetry->setPolicy(ComboBoxWidget::OrderPolicy::bottom);
}

void BrushToolWidget::setupLayout() {
  auto *layout = new QHBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  
  layout->addWidget(makeLabel(this, "Radius: "));
  layout->addWidget(radius);
  layout->addWidget(makeLabel(this, " Symmetry: "));
  layout->addWidget(symmetry);
}

void BrushToolWidget::connectSignals(BrushTool *tool) {
  CONNECT_LAMBDA(radius, valueChanged, [tool](const int value) {
    tool->setRadius(value);
  });
  CONNECT_LAMBDA(symmetry, currentTextChanged, [tool](const QString &str) {
    tool->setMode(symmetryFromStr(str));
  });
}
