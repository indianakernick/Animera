//
//  palette widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "palette widget.hpp"

#include <cmath>
#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include "color handle.hpp"
#include "color convert.hpp"
#include "radio button widget.hpp"
#include <QtWidgets/qgridlayout.h>

namespace {

constexpr int quantColor(const double size, const int color) {
  return 255 * color / size + 0.5;
}

constexpr QRgb quantColor(const int size, const int r, const int g, const int b) {
  return qRgb(quantColor(size, r), quantColor(size, g), quantColor(size, b));
}

constexpr QRgb quantGray(const int size, const int y) {
  const int gray = quantColor(size, y);
  return qRgb(gray, gray, gray);
}

const std::vector<QRgb> default_palette = {
  quantColor(4, 4, 0, 0),
  quantColor(4, 4, 1, 0),
  quantColor(4, 4, 2, 0),
  quantColor(4, 4, 3, 0),
  quantColor(4, 4, 4, 0),
  quantColor(4, 3, 4, 0),
  quantColor(4, 2, 4, 0),
  quantColor(4, 1, 4, 0),
  
  quantColor(4, 0, 4, 0),
  quantColor(4, 0, 4, 1),
  quantColor(4, 0, 4, 2),
  quantColor(4, 0, 4, 3),
  quantColor(4, 0, 4, 4),
  quantColor(4, 0, 3, 4),
  quantColor(4, 0, 2, 4),
  quantColor(4, 0, 1, 4),
  
  quantColor(4, 0, 0, 4),
  quantColor(4, 1, 0, 4),
  quantColor(4, 2, 0, 4),
  quantColor(4, 3, 0, 4),
  quantColor(4, 4, 0, 4),
  quantColor(4, 4, 0, 3),
  quantColor(4, 4, 0, 2),
  quantColor(4, 4, 0, 1),
  
  quantGray(8, 0),
  quantGray(8, 1),
  quantGray(8, 2),
  quantGray(8, 3),
  quantGray(8, 4),
  quantGray(8, 5),
  quantGray(8, 6),
  quantGray(8, 7),
  
  quantGray(8, 8),
};

}

constexpr int tile_size = 13_px;

class PaletteColorWidget final : public RadioButtonWidget, public ColorHandle {
  Q_OBJECT
  
public:
  PaletteColorWidget(QWidget *parent, QRgb &color, const uint32_t index)
    : RadioButtonWidget{parent}, color{color}, index{index} {
    setFixedSize(tile_size, tile_size);
    CONNECT(this, toggled, this, click);
  }

Q_SIGNALS:
  void attachColor(ColorHandle *);
  void setColor(QRgb);

private Q_SLOTS:
  void click() {
    Q_EMIT attachColor(this);
  }

private:
  QRgb &color;
  uint32_t index;

  void paintChecker(QPainter &painter) {
    constexpr int bord = glob_border_width;
    constexpr int half = (tile_size - bord) / 2;
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
      tile_size - glob_border_width, tile_size - glob_border_width
    );
  }
  
  void paintBorder(QPainter &painter) {
    painter.setBrush(glob_border_color);
    painter.drawRect(0, 0, tile_size, glob_border_width);
    painter.drawRect(0, glob_border_width, glob_border_width, tile_size);
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
    : QWidget{parent}, palette(256) {
    setupLayout();
    std::copy(default_palette.cbegin(), default_palette.cend(), palette.begin());
  }

Q_SIGNALS:
  void attachColor(ColorHandle *);
  void setColor(QRgb);

private:
  std::vector<PaletteColorWidget *> colors;
  std::vector<QRgb> palette;
  
  void setupLayout() {
    const uint32_t width = 8;
    const uint32_t height = 32;
    
    QGridLayout *grid = new QGridLayout{this};
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);
    
    for (uint32_t y = 0; y != height; ++y) {
      for (uint32_t x = 0; x != width; ++x) {
        const uint32_t index = y * width + x;
        auto *colorWidget = new PaletteColorWidget{this, palette[index], index};
        CONNECT(colorWidget, attachColor, this, attachColor);
        CONNECT(colorWidget, setColor, this, setColor);
        colors.push_back(colorWidget);
        grid->addWidget(colorWidget, y, x);
      }
    }
    
    setFixedSize(width * tile_size + glob_border_width, height * tile_size + glob_border_width);
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
  setFixedWidth(pick_svgraph_rect.widget().width() + 2_px);
  setAlignment(Qt::AlignHCenter);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  CONNECT(table, attachColor, this, attachColor);
  CONNECT(table, setColor, this, setColor);
}

#include "palette widget.moc"
