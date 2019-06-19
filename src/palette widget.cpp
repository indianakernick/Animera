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

namespace {

// https://www.quackit.com/css/color/charts/css_color_names_chart.cfm

const std::vector<QRgb> css_colors = {
  // Reds
  qRgb(205, 92, 92),   // Indian Red
  qRgb(240, 128, 128), // Light Coral
  qRgb(250, 128, 114), // Salmon
  qRgb(233, 150, 122), // Dark Salmon
  qRgb(255, 160, 122), // Light Salmon
  qRgb(220, 20, 60),   // Crimson
  qRgb(255, 0, 0),     // Red
  qRgb(178, 34, 34),   // Fire Brick
  qRgb(139, 0, 0),     // Dark Red
  
  // Pinks
  qRgb(255, 192, 203), // Pink
  qRgb(255, 182, 193), // Light Pink
  qRgb(255, 105, 180), // Hot Pink
  qRgb(255, 20, 147),  // Deep Pink
  qRgb(199, 21, 133),  // Medium Violet Red
  qRgb(219, 112, 147), // Pale Violet Red
  
  // Oranges
  qRgb(255, 127, 80),  // Coral
  qRgb(255, 99, 71),   // Tomato
  qRgb(255, 69, 0),    // Orange Red
  qRgb(255, 140, 0),   // Dark Orange
  qRgb(255, 165, 0),   // Orange
  
  // Yellows
  qRgb(255, 215, 0),   // Gold
  qRgb(255, 255, 0),   // Yellow
  qRgb(255, 255, 224), // Light Yellow
  qRgb(255, 250, 205), // Lemon Chiffon
  qRgb(250, 250, 210), // Light Goldenrod Yellow
  qRgb(255, 239, 213), // Papaya Whip
  qRgb(255, 228, 181), // Moccasin
  qRgb(255, 218, 185), // Peach Puff
  qRgb(238, 232, 170), // Pale Goldenrod
  qRgb(240, 230, 140), // Khaki
  qRgb(189, 183, 107), // Dark Khaki
  
  // Purples
  qRgb(230, 230, 250), // Lavender
  qRgb(216, 191, 216), // Thistle
  qRgb(221, 160, 221), // Plum
  qRgb(238, 130, 238), // Violet
  qRgb(218, 112, 214), // Orchid
  qRgb(255, 0, 255),   // Magenta/Fuchsia
  qRgb(186, 85, 211),  // Medium Orchid
  qRgb(147, 112, 219), // Medium Purple
  qRgb(138, 43, 226),  // Blue Violet
  qRgb(148, 0, 211),   // Dark Violet
  qRgb(153, 50, 204),  // Dark Orchid
  qRgb(139, 0, 139),   // Dark Magenta
  qRgb(128, 0, 128),   // Purple
  qRgb(102, 51, 153),  // Rebecca Purple
  qRgb(75, 0, 130),    // Indigo
  qRgb(123, 104, 238), // Medium Slate Blue
  qRgb(106, 90, 205),  // Slate Blue
  qRgb(72, 61, 139),   // Dark Slate Blue
  
  // Greens
  qRgb(173, 255, 47),  // Green Yellow
  qRgb(127, 255, 0),   // Chartreuse
  qRgb(124, 252, 0),   // Lawn Green
  qRgb(0, 255, 0),     // Lime
  qRgb(50, 205, 50),   // Lime Green
  qRgb(152, 251, 152), // Pale Green
  qRgb(144, 238, 144), // Light Green
  qRgb(0, 250, 154),   // Medium Spring Green
  qRgb(0, 255, 127),   // Spring Green
  qRgb(60, 179, 113),  // Medium Sea Green
  qRgb(46, 139, 87),   // Sea Green
  qRgb(34, 139, 34),   // Forest Green
  qRgb(0, 128, 0),     // Green
  qRgb(0, 100, 0),     // Dark Green
  qRgb(154, 205, 50),  // Yellow Green
  qRgb(107, 142, 35),  // Olive Drab
  qRgb(128, 128, 0),   // Olive
  qRgb(85, 107, 47),   // Dark Olive Green
  qRgb(102, 205, 170), // Medium Aquamarine
  qRgb(143, 188, 143), // Dark Sea Green
  qRgb(32, 178, 170),  // Light Sea Green
  qRgb(0, 139, 139),   // Dark Cyan
  qRgb(0, 128, 128),   // Teal
  
  // Blues/Cyans
  qRgb(0, 255, 255),   // Cyan/Aqua
  qRgb(224, 255, 255), // Light Cyan
  qRgb(175, 238, 238), // Pale Turquoise
  qRgb(127, 255, 212), // Aquamarine
  qRgb(64, 224, 208),  // Turquoise
  qRgb(72, 209, 204),  // Medium Torquoise
  qRgb(0, 206, 209),   // Dark Torquoise
  qRgb(95, 158, 160),  // Cadet Blue
  qRgb(70, 130, 180),  // Steel Blue
  qRgb(176, 196, 222), // Light Steel Blue
  qRgb(176, 224, 230), // Powder Blue
  qRgb(173, 216, 230), // Light Blue
  qRgb(135, 206, 235), // Sky Blue
  qRgb(0, 191, 255),   // Deep Sky Blue
  qRgb(30, 144, 255),  // Dodger Blue
  qRgb(100, 149, 237), // Cornflower Blue
  qRgb(65, 105, 225),  // Royal Blue
  qRgb(0, 0, 255),     // Blue/Medium Blue
  qRgb(0, 0, 139),     // Dark Blue
  qRgb(0, 0, 128),     // Navy
  qRgb(25, 25, 112),   // Midnight Blue
  
  // Browns
  qRgb(255, 248, 220), // Cornsilk
  qRgb(255, 235, 205), // Blanched Almond
  qRgb(255, 228, 196), // Bisque
  qRgb(255, 222, 173), // Navajo White
  qRgb(245, 222, 179), // Wheat
  qRgb(222, 184, 135), // Burly Wood
  qRgb(210, 180, 140), // Tan
  qRgb(188, 143, 143), // Rosy Brown
  qRgb(244, 164, 96),  // Sandy Brown
  qRgb(218, 165, 32),  // Goldenrod
  qRgb(184, 134, 11),  // Dark Goldenrod
  qRgb(205, 133, 63),  // Peru
  qRgb(210, 105, 30),  // Chocolate
  qRgb(139, 69, 19),   // Saddle Brown
  qRgb(160, 82, 45),   // Sienna
  qRgb(165, 42, 42),   // Brown
  qRgb(128, 0, 0),     // Maroon
  
  // Whites
  qRgb(255, 255, 255), // White
  qRgb(255, 250, 250), // Snow
  qRgb(240, 255, 240), // Honeydew
  qRgb(245, 255, 250), // Mint Cream
  qRgb(240, 255, 255), // Azure
  qRgb(240, 248, 255), // Alice Blue
  qRgb(248, 248, 255), // Ghost White
  qRgb(245, 245, 245), // White Smoke
  qRgb(255, 245, 245), // Seashell
  qRgb(245, 245, 220), // Beige
  qRgb(253, 245, 230), // Old Lace
  qRgb(255, 250, 240), // Floral White
  qRgb(255, 255, 240), // Ivory
  qRgb(250, 235, 215), // Antique white
  qRgb(250, 240, 230), // Linen
  qRgb(255, 240, 245), // Lavender Blush
  qRgb(255, 228, 225), // Misty Rose
  
  // Grays
  qRgb(220, 220, 220), // Gainsboro
  qRgb(211, 211, 211), // Light Gray/Light Grey
  qRgb(192, 192, 192), // Silver
  qRgb(169, 169, 169), // Dark Gray/Dark Grey
  qRgb(128, 128, 128), // Gray/Grey
  qRgb(105, 105, 105), // Dim Gray/Dim Grey
  qRgb(119, 136, 153), // Light Slate Gray/Light Slate Grey
  qRgb(112, 128, 144), // Slate Gray/Slate Grey
  qRgb(47, 79, 79),    // Dark Slate Gray/Dark Slate Grey
  qRgb(0, 0, 0),       // Black
};

}

constexpr int tile_size = 13_px;

class PaletteColorWidget final : public RadioButtonWidget, public ColorHandle {
  Q_OBJECT
  
public:
  PaletteColorWidget(QWidget *parent, std::vector<QRgb> &palette, const uint32_t index)
    : RadioButtonWidget{parent}, palette{palette}, index{index} {
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
  // @TODO Span
  std::vector<QRgb> &palette;
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
    painter.setBrush(QColor::fromRgba(palette[index]));
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
      if (qGray(palette[index]) < 128) {
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
      Q_EMIT setColor(palette[index]);
    }
  }
  
  QRgb getInitialColor() const override {
    return palette[index];
  }
  void changeColor(const QRgb newColor) override {
    palette[index] = newColor;
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
    std::copy(css_colors.cbegin(), css_colors.cend(), palette.begin());
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
        auto *colorWidget = new PaletteColorWidget{this, palette, index};
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
