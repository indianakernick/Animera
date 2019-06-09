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
#include "color convert.hpp"
#include "radio button widget.hpp"
#include <QtWidgets/qgridlayout.h>

constexpr int tile_size = 13_px;

class PaletteColorWidget final : public RadioButtonWidget, public ColorHandle {
  Q_OBJECT
  
public:
  PaletteColorWidget(QWidget *parent, const QRgb color, const uint8_t index)
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
  // this should probably be a pointer
  QRgb color;
  uint8_t index;

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor::fromRgba(color));
    painter.drawRect(
      glob_border_width, glob_border_width,
      tile_size - glob_border_width, tile_size - glob_border_width
    );
    painter.setBrush(glob_border_color);
    painter.drawRect(0, 0, tile_size, glob_border_width);
    painter.drawRect(0, glob_border_width, glob_border_width, tile_size);
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
    return "Palette " + QString::number(static_cast<int>(index));
  }
  void detach() override {
    uncheck();
  }
};

class PaletteTableWidget final : public QWidget {
  Q_OBJECT
  
public:
  explicit PaletteTableWidget(QWidget *parent)
    : QWidget{parent} {
    setupLayout();
  }

Q_SIGNALS:
  void attachColor(ColorHandle *);
  void setColor(QRgb);

private:
  std::vector<PaletteColorWidget *> colors;
  
  void setupLayout() {
    const int width = 8;
    const int height = 32;
    
    QGridLayout *grid = new QGridLayout{this};
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);
    
    for (int y = 0; y != height; ++y) {
      for (int x = 0; x != width; ++x) {
        const QRgb color = qRgb(x * 255 / (width - 1), y * 255 / (height - 1), 127);
        const uint8_t index = y * width + x;
        auto *colorWidget = colors.emplace_back(new PaletteColorWidget{this, color, index});
        CONNECT(colorWidget, attachColor, this, attachColor);
        CONNECT(colorWidget, setColor, this, setColor);
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
