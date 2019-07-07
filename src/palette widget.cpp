//
//  palette widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "palette widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include "color handle.hpp"
#include "radio button widget.hpp"
#include <QtWidgets/qgridlayout.h>

class PaletteColorWidget final : public RadioButtonWidget, public ColorHandle {
  Q_OBJECT
  
public:
  PaletteColorWidget(QWidget *parent, QRgb &color, const int index)
    : RadioButtonWidget{parent}, color{color}, index{index} {
    setFixedSize(pal_tile_size, pal_tile_size);
    CONNECT(this, toggled, this, click);
  }

Q_SIGNALS:
  void attachColor(ColorHandle *);
  void setColor(QRgb);
  void paletteColorChanged();

private Q_SLOTS:
  void click() {
    Q_EMIT attachColor(this);
  }

private:
  QRgb &color;
  int index;

  void paintChecker(QPainter &painter) {
    constexpr int bord = glob_border_width;
    constexpr int half = (pal_tile_size - bord) / 2;
    painter.setBrush(QColor{edit_checker_a});
    painter.drawRect(bord, bord, half, half);
    painter.drawRect(bord + half, bord + half, half, half);
    painter.setBrush(QColor{edit_checker_b});
    painter.drawRect(bord + half, bord, half, half);
    painter.drawRect(bord, bord + half, half, half);
  }
  
  void paintColor(QPainter &painter) {
    painter.setBrush(QColor::fromRgba(color));
    painter.drawRect(
      glob_border_width, glob_border_width,
      pal_tile_size - glob_border_width, pal_tile_size - glob_border_width
    );
  }
  
  void paintBorder(QPainter &painter) {
    painter.setBrush(glob_border_color);
    painter.drawRect(0, 0, pal_tile_size, glob_border_width);
    painter.drawRect(0, glob_border_width, glob_border_width, pal_tile_size);
  }
  
  void paintSelect(QPainter &painter) {
    if (isChecked()) {
      if (qGray(color) < 128) {
        painter.setBrush(QColor{255, 255, 255});
      } else {
        painter.setBrush(QColor{0, 0, 0});
      }
      painter.drawRect(
        glob_border_width, glob_border_width,
        (width() - glob_border_width) / 2,
        (height() - glob_border_width)
      );
    }
  }

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.setPen(Qt::NoPen);
    paintChecker(painter);
    paintColor(painter);
    paintBorder(painter);
    paintSelect(painter);
  }
  
  void mousePressEvent(QMouseEvent *event) override {
    RadioButtonWidget::mousePressEvent(event);
    if (event->button() == Qt::RightButton) {
      Q_EMIT setColor(color);
    }
  }
  
  QRgb getInitialColor() const override {
    return color;
  }
  void changeColor(const QRgb newColor) override {
    color = newColor;
    repaint();
    Q_EMIT paletteColorChanged();
  }
  QString getName() const override {
    return "Palette " + QString::number(index);
  }
  void detach() override {
    uncheck();
  }
};

class PaletteTableWidget final : public QWidget {
  Q_OBJECT
  
public:
  explicit PaletteTableWidget(QWidget *parent)
    : QWidget{parent} {}

public Q_SLOTS:
  void setPalette(const PaletteSpan newPalette) {
    palette = newPalette;
    setupLayout();
  }

Q_SIGNALS:
  void attachColor(ColorHandle *);
  void setColor(QRgb);
  void paletteChanged(PaletteSpan);
  void paletteColorChanged();

private:
  std::vector<PaletteColorWidget *> colors;
  PaletteSpan palette;
  
  void setupLayout() {
    QGridLayout *grid = new QGridLayout{this};
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);
    
    for (int y = 0; y != pal_height; ++y) {
      for (int x = 0; x != pal_width; ++x) {
        const int index = y * pal_width + x;
        auto *colorWidget = new PaletteColorWidget{this, palette[index], index};
        CONNECT(colorWidget, attachColor,         this, attachColor);
        CONNECT(colorWidget, setColor,            this, setColor);
        CONNECT(colorWidget, paletteColorChanged, this, paletteColorChanged);
        colors.push_back(colorWidget);
        grid->addWidget(colorWidget, y, x);
      }
    }
    
    setFixedSize(
      pal_width * pal_tile_size + glob_border_width,
      pal_height * pal_tile_size + glob_border_width
    );
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.setPen(Qt::NoPen);
    painter.setBrush(glob_border_color);
    painter.drawRect(width() - glob_border_width, 0, glob_border_width, height());
    painter.drawRect(0, height() - glob_border_width, width() - glob_border_width, glob_border_width);
  }
};

PaletteWidget::PaletteWidget(QWidget *parent)
  : QScrollArea{parent}, table{new PaletteTableWidget{this}} {
  setWidget(table);
  setFrameShape(NoFrame);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  setFixedWidth(pick_svgraph_rect.widget().width() + 2 * glob_border_width);
  setMinimumHeight(pal_tile_size + glob_border_width);
  setAlignment(Qt::AlignHCenter);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  CONNECT(table, attachColor,         this, attachColor);
  CONNECT(table, setColor,            this, setColor);
  CONNECT(table, paletteColorChanged, this, paletteColorChanged);
}

void PaletteWidget::initCanvas() {
  
}

void PaletteWidget::setPalette(PaletteSpan palette) {
  table->setPalette(palette);
}

#include "palette widget.moc"
