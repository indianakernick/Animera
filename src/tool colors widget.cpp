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
  ActiveColorWidget(QWidget *parent, const QString &name, QRgb &color, const Format format)
    : RadioButtonWidget{parent}, name{name}, color{color}, format{format} {
    setFixedSize(tool_color_rect.widget().size());
  }

public Q_SLOTS:
  void setPalette(const PaletteCSpan newPalette) {
    palette = newPalette;
  }
  void setIndex(const int index) {
    if (isChecked()) {
      color = index;
      repaint();
      Q_EMIT colorChanged();
    }
  }
  void changePaletteColors() {
    if (format == Format::palette) {
      repaint();
    }
  }

Q_SIGNALS:
  void colorChanged();
  
private:
  QString name;
  QRgb &color;
  Format format;
  PaletteCSpan palette;
  
  QColor getQColor() const {
    switch (format) {
      case Format::rgba:
        return QColor::fromRgba(color);
      case Format::palette:
        return QColor::fromRgba(palette[color]);
      case Format::gray:
        const int gray = color;
        return QColor{gray, gray, gray};
    }
  }
  
  void paintEvent(QPaintEvent *) override {
    // @TODO consider baking checkerboard and border
    QPainter painter{this};
    paintChecker(painter, tool_color_rect, tool_color_tiles);
    if (isChecked()) {
      painter.fillRect(active_color_rect.inner(), getQColor());
      paintBorder(painter, active_color_rect, glob_border_color);
    } else {
      painter.fillRect(tool_color_rect.inner(), getQColor());
      paintBorder(painter, tool_color_rect, glob_border_color);
    }
  }
  
  QRgb getInitialColor() const override {
    return color;
  }
  void changeColor(const QRgb newColor) override {
    color = newColor;
    repaint();
    Q_EMIT colorChanged();
  }
  QString getName() const override {
    return name;
  }
  void detach() override {
    uncheck();
  }
};

ToolColorsWidget::ToolColorsWidget(QWidget *parent)
  : QWidget{parent} {
  setFixedSize(tool_colors_rect.widget().size());
}

void ToolColorsWidget::initCanvas(const Format format) {
  initColors(format);
  primary = new ActiveColorWidget{this, "Primary", colors.primary, format};
  secondary = new ActiveColorWidget{this, "Secondary", colors.secondary, format};
  erase = new ActiveColorWidget{this, "Erase", colors.erase, format};
  setupLayout();
  connectSignals(format);
  Q_EMIT colorsChanged(colors);
}

void ToolColorsWidget::setPalette(const PaletteCSpan palette) {
  primary->setPalette(palette);
  secondary->setPalette(palette);
  erase->setPalette(palette);
  primary->click();
}

void ToolColorsWidget::setIndex(const int index) {
  primary->setIndex(index);
  secondary->setIndex(index);
  erase->setIndex(index);
}

void ToolColorsWidget::changePaletteColors() {
  primary->changePaletteColors();
  secondary->changePaletteColors();
  erase->changePaletteColors();
}

void ToolColorsWidget::changeColors() {
  Q_EMIT colorsChanged(colors);
}

void ToolColorsWidget::setupLayout() {
  QHBoxLayout *layout = new QHBoxLayout{this};
  setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  
  layout->addWidget(primary);
  layout->addWidget(secondary);
  layout->addWidget(erase);
}

template <auto Color>
void ToolColorsWidget::toggleColor(const bool checked) {
  if (checked) {
    Q_EMIT attachColor(this->*Color);
    repaint();
  }
}

template <auto Color>
void ToolColorsWidget::toggleIndex(const bool checked) {
  if (checked) {
    Q_EMIT attachIndex(colors.*Color);
    repaint();
  }
}

void ToolColorsWidget::connectSignals(const Format format) {
  CONNECT(primary, colorChanged, this, changeColors);
  CONNECT(secondary, colorChanged, this, changeColors);
  CONNECT(erase, colorChanged, this, changeColors);
  
  if (format == Format::palette) {
    CONNECT(primary, toggled, this, toggleIndex<&ToolColors::primary>);
    CONNECT(secondary, toggled, this, toggleIndex<&ToolColors::secondary>);
    CONNECT(erase, toggled, this, toggleIndex<&ToolColors::erase>);
  } else {
    CONNECT(primary, toggled, this, toggleColor<&ToolColorsWidget::primary>);
    CONNECT(secondary, toggled, this, toggleColor<&ToolColorsWidget::secondary>);
    CONNECT(erase, toggled, this, toggleColor<&ToolColorsWidget::erase>);
  }
}

void ToolColorsWidget::initColors(const Format format) {
  switch (format) {
    case Format::rgba:
      colors.primary = qRgba(255, 0, 0, 255);
      colors.secondary = qRgba(0, 0, 255, 255);
      colors.erase = qRgba(0, 0, 0, 0);
      break;
    case Format::palette:
      colors.primary = 1;
      colors.secondary = 2;
      colors.erase = 0;
      break;
    case Format::gray:
      colors.primary = 255;
      colors.secondary = 128;
      colors.erase = 0;
      break;
  }
}

#include "tool colors widget.moc"
