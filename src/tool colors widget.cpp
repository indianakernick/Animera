//
//  tool colors widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 30/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool colors widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qpainter.h>
#include "color handle.hpp"
#include "color convert.hpp"
#include "widget painting.hpp"
#include <QtWidgets/qboxlayout.h>
#include "radio button widget.hpp"

class ActiveColorWidget final : public RadioButtonWidget, public ColorHandle {
  Q_OBJECT
  
public:
  ActiveColorWidget(QWidget *parent, const QString &name, QRgb *color)
    : RadioButtonWidget{parent}, name{name}, color{color} {
    setCursor(Qt::PointingHandCursor);
    setCheckable(true);
    setAutoExclusive(true);
    setFixedSize(tool_color_rect.widget().size());
  }
  
  QRgb getInitialColor() const override {
    return *color;
  }
  
  void changeColor(const QRgb newColor) override {
    *color = newColor;
    repaint();
    Q_EMIT colorChanged();
  }
  
  QString getName() const override {
    return name;
  }

Q_SIGNALS:
  void colorChanged();
  
private:
  QString name;
  QRgb *color;
  
  void paintEvent(QPaintEvent *) override {
    // @TODO consider baking checkerboard and border
    QPainter painter{this};
    paintChecker(painter, tool_color_rect, tool_color_tiles);
    if (isChecked()) {
      painter.fillRect(active_color_rect.inner(), QColor::fromRgba(*color));
      paintBorder(painter, active_color_rect, glob_border_color);
    } else {
      painter.fillRect(tool_color_rect.inner(), QColor::fromRgba(*color));
      paintBorder(painter, tool_color_rect, glob_border_color);
    }
  }
};

ToolColorsWidget::ToolColorsWidget(QWidget *parent)
  : QWidget{parent},
    primary{new ActiveColorWidget{this, "Primary", &colors.primary}},
    secondary{new ActiveColorWidget{this, "Secondary", &colors.secondary}},
    erase{new ActiveColorWidget{this, "Erase", &colors.erase}} {
  setFixedSize(tool_colors_rect.widget().size());
  colors.primary = qRgba(255, 0, 0, 255);
  colors.secondary = qRgba(0, 0, 255, 255);
  colors.erase = qRgba(0, 0, 0, 0);
  setupLayout();
  connectSignals();
}

void ToolColorsWidget::attachPrimary() {
  // @TODO should I do a QTimer::singleShot instead
  primary->click();
  Q_EMIT colorsChanged(colors);
}

void ToolColorsWidget::setupLayout() {
  QHBoxLayout *layout = new QHBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  
  layout->addWidget(primary);
  layout->addWidget(secondary);
  layout->addWidget(erase);
  
  setLayout(layout);
}

void ToolColorsWidget::connectSignals() {
  CONNECT(primary, colorChanged, this, changeColors);
  CONNECT(secondary, colorChanged, this, changeColors);
  CONNECT(erase, colorChanged, this, changeColors);
  
  connect(primary, &QAbstractButton::toggled, [this](const bool checked) {
    if (checked) {
      Q_EMIT attachColor(primary);
      repaint();
    }
  });
  connect(secondary, &QAbstractButton::toggled, [this](const bool checked) {
    if (checked) {
      Q_EMIT attachColor(secondary);
      repaint();
    }
  });
  connect(erase, &QAbstractButton::toggled, [this](const bool checked) {
    if (checked) {
      Q_EMIT attachColor(erase);
      repaint();
    }
  });
}

void ToolColorsWidget::paintEvent(QPaintEvent *) {
  
}

void ToolColorsWidget::changeColors() {
  Q_EMIT colorsChanged(colors);
}

#include "tool colors widget.moc"
