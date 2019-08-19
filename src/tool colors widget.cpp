//
//  tool colors widget.cpp
//  Animera
//
//  Created by Indi Kernick on 30/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool colors widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "formats.hpp"
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
    if (format == Format::index) {
      CONNECT(this, toggled, this, attachIndex);
    } else {
      CONNECT(this, toggled, this, attachColor);
    }
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
    if (format == Format::index) {
      repaint();
    }
  }

private Q_SLOTS:
  void attachColor(const bool checked) {
    if (checked) {
      Q_EMIT shouldAttachColor(this);
      repaint();
    }
  }
  void attachIndex(const bool checked) {
    if (checked) {
      Q_EMIT shouldAttachIndex(color);
      repaint();
    }
  }

Q_SIGNALS:
  void colorChanged();
  void shouldAttachColor(ColorHandle *);
  void shouldAttachIndex(int);
  
private:
  QString name;
  QRgb &color;
  Format format;
  PaletteCSpan palette;
  
  QColor getQColor() const {
    switch (format) {
      case Format::rgba:
        return QColor::fromRgba(color);
      case Format::index:
        return QColor::fromRgba(palette[color]);
      case Format::gray:
        const int gray = FormatYA::toGray(color);
        const int alpha = FormatYA::toAlpha(color);
        return QColor{gray, gray, gray, alpha};
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
  colors = getInitialColors(format);
  widgets[0] = new ActiveColorWidget{this, "Primary", colors.primary, format};
  widgets[1] = new ActiveColorWidget{this, "Secondary", colors.secondary, format};
  widgets[2] = new ActiveColorWidget{this, "Erase", colors.erase, format};
  setupLayout();
  connectSignals();
  Q_EMIT colorsChanged(colors);
}

void ToolColorsWidget::setPalette(const PaletteCSpan palette) {
  for (auto *widget : widgets) {
    widget->setPalette(palette);
  }
  widgets[0]->click();
}

void ToolColorsWidget::setIndex(const int index) {
  for (auto *widget : widgets) {
    widget->setIndex(index);
  }
}

void ToolColorsWidget::changePaletteColors() {
  for (auto *widget : widgets) {
    widget->changePaletteColors();
  }
}

void ToolColorsWidget::changeColors() {
  Q_EMIT colorsChanged(colors);
}

ToolColors ToolColorsWidget::getInitialColors(const Format format) {
  switch (format) {
    case Format::rgba:
      return {qRgb(255, 0, 0), qRgb(0, 0, 255), 0};
    case Format::index:
      return {1, 2, 0};
    case Format::gray:
      return {
        FormatYA::toPixel(255, 255),
        FormatYA::toPixel(0, 255),
        FormatYA::toPixel(0, 0)
      };
  }
}

void ToolColorsWidget::setupLayout() {
  auto *layout = new QHBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  for (auto *widget : widgets) {
    layout->addWidget(widget);
  }
}

void ToolColorsWidget::connectSignals() {
  for (auto *widget : widgets) {
    CONNECT(widget, colorChanged,      this, changeColors);
    CONNECT(widget, shouldAttachColor, this, shouldAttachColor);
    CONNECT(widget, shouldAttachIndex, this, shouldAttachIndex);
  }
}

#include "tool colors widget.moc"
