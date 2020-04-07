//
//  tool widgets.cpp
//  Animera
//
//  Created by Indiana Kernick on 24/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
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

gfx::CircleShape shapeFromStr(const QString &str) {
         if (str == "1x1") {
    return gfx::CircleShape::c1x1;
  } else if (str == "1x2") {
    return gfx::CircleShape::c1x2;
  } else if (str == "2x1") {
    return gfx::CircleShape::c2x1;
  } else if (str == "2x2") {
    return gfx::CircleShape::c2x2;
  } else Q_UNREACHABLE();
}

ComboBoxWidget *makeSymmetry(QWidget *parent) {
  auto *symmetry = new ComboBoxWidget{parent, 10};
  symmetry->clearWithItem("None");
  symmetry->addItem("Horizontal");
  symmetry->addItem("Vertical");
  symmetry->addItem("Both");
  symmetry->setPolicy(ComboBoxWidget::OrderPolicy::bottom);
  return symmetry;
}

ComboBoxWidget *makeShape(QWidget *parent) {
  auto *shape = new ComboBoxWidget{parent, 3};
  shape->clearWithItem("1x1");
  shape->addItem("1x2");
  shape->addItem("2x1");
  shape->addItem("2x2");
  shape->setPolicy(ComboBoxWidget::OrderPolicy::bottom);
  return shape;
}

QHBoxLayout *makeLayout(QWidget *parent) {
  auto *layout = new QHBoxLayout{parent};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  return layout;
}

}

void BrushToolWidget::createWidgets() {
  radius = new NumberInputWidget{this, pick_number_rect, brsh_radius};
  symmetry = makeSymmetry(this);
}

void BrushToolWidget::setupLayout() {
  QHBoxLayout *layout = makeLayout(this);
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

void LineToolWidget::createWidgets() {
  radius = new NumberInputWidget{this, pick_number_rect, line_radius};
}

void LineToolWidget::setupLayout() {
  QHBoxLayout *layout = makeLayout(this);
  layout->addWidget(makeLabel(this, "Radius: "));
  layout->addWidget(radius);
}

void LineToolWidget::connectSignals(LineTool *tool) {
  CONNECT_LAMBDA(radius, valueChanged, [tool](const int value) {
    tool->setRadius(value);
  });
}

void StrokedCircleToolWidget::createWidgets() {
  thickness = new NumberInputWidget{this, pick_number_rect, circ_thick};
  shape = makeShape(this);
}

void StrokedCircleToolWidget::setupLayout() {
  QHBoxLayout *layout = makeLayout(this);
  layout->addWidget(makeLabel(this, "Thickness: "));
  layout->addWidget(thickness);
  layout->addWidget(makeLabel(this, " Shape: "));
  layout->addWidget(shape);
}

void StrokedCircleToolWidget::connectSignals(StrokedCircleTool *tool) {
  CONNECT_LAMBDA(thickness, valueChanged, [tool](const int value) {
    tool->setThick(value);
  });
  CONNECT_LAMBDA(shape, currentTextChanged, [tool](const QString &str) {
    tool->setShape(shapeFromStr(str));
  });
}

void FilledCircleToolWidget::createWidgets() {
  shape = makeShape(this);
}

void FilledCircleToolWidget::setupLayout() {
  QHBoxLayout *layout = makeLayout(this);
  layout->addWidget(makeLabel(this, "Shape: "));
  layout->addWidget(shape);
}

void FilledCircleToolWidget::connectSignals(FilledCircleTool *tool) {
  CONNECT_LAMBDA(shape, currentTextChanged, [tool](const QString &str) {
    tool->setShape(shapeFromStr(str));
  });
}

void StrokedRectangleToolWidget::createWidgets() {
  thickness = new NumberInputWidget{this, pick_number_rect, rect_thick};
}

void StrokedRectangleToolWidget::setupLayout() {
  QHBoxLayout *layout = makeLayout(this);
  layout->addWidget(makeLabel(this, "Thickness: "));
  layout->addWidget(thickness);
}

void StrokedRectangleToolWidget::connectSignals(StrokedRectangleTool *tool) {
  CONNECT_LAMBDA(thickness, valueChanged, [tool](const int value) {
    tool->setThick(value);
  });
}
