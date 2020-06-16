//
//  palette widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 3/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "palette widget.hpp"

#include "connect.hpp"
#include "status msg.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include "color handle.hpp"
#include "config colors.hpp"
#include "config geometry.hpp"
#include "widget painting.hpp"
#include "graphics convert.hpp"

class PaletteTableWidget final : public QWidget, public ColorHandle {
  Q_OBJECT
  
public:
  PaletteTableWidget(QWidget *parent, const Format format)
    : QWidget{parent}, format{format} {
    setFixedSize(
      pal_width * pal_tile_stride + glob_border_width,
      pal_height * pal_tile_stride + glob_border_width
    );
    initSelect();
    initBackground();
    setMouseTracking(true);
  }

public Q_SLOTS:
  void setPalette(const PaletteSpan newPalette) {
    palette = newPalette;
    update();
  }
  
  void attachIndex(const PixelIndex newIndex) {
    Q_EMIT shouldAttachColor(nullptr);
    index = newIndex;
    Q_EMIT shouldAttachColor(this);
    selected = true;
    update(QRect{entryPos(index), toSize(pal_tile_size)});
  }
  
  void updatePalette() {
    Q_EMIT shouldAttachColor(this);
    Q_EMIT paletteColorChanged();
    update();
  }

Q_SIGNALS:
  void shouldAttachColor(ColorHandle *);
  void shouldSetColor(PixelVar);
  void shouldSetIndex(PixelIndex);
  void paletteChanged(PaletteSpan);
  void paletteColorChanged();
  void shouldShowNorm(std::string_view);

private:
  Format format;
  PaletteSpan palette;
  PixelIndex index;
  QPixmap selectBlack;
  QPixmap selectWhite;
  QPixmap background;
  bool selected = false;

  void initSelect() {
    QBitmap bitmap{":/Color Picker/palette select.png"};
    bitmap = bitmap.scaled(bitmap.size() * glob_scale);
    selectBlack = bakeColoredBitmap(bitmap, QColor{0, 0, 0});
    selectWhite = bakeColoredBitmap(bitmap, QColor{255, 255, 255});
  }
  
  QPixmap createCheckers() {
    constexpr int size = pal_tile_size / glob_scale;
    constexpr int half = size / 2;
    
    QPixmap checkers{size, size};
    QPainter painter{&checkers};
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setPen(Qt::NoPen);
    
    painter.setBrush(QColor{edit_checker_a});
    painter.drawRect(0, 0, half, half);
    painter.drawRect(half, half, half, half);
    painter.setBrush(QColor{edit_checker_b});
    painter.drawRect(half, 0, half, half);
    painter.drawRect(0, half, half, half);
    
    return checkers;
  }
  
  void initBackground() {
    constexpr int stride = pal_tile_stride / glob_scale;
    constexpr int border = glob_border_width / glob_scale;
  
    background = QPixmap{
      pal_width * stride + border,
      pal_height * stride + border
    };
    
    QPainter painter{&background};
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setPen(Qt::NoPen);
    
    painter.setBrush(glob_border_color);
    for (int y = 0; y <= pal_height; ++y) {
      painter.drawRect(0, y * stride, background.width(), border);
    }
    for (int x = 0; x <= pal_width; ++x) {
      painter.drawRect(x * stride, 0, border, background.height());
    }
    
    QPixmap checkers = createCheckers();
    for (int y = 0; y != pal_height; ++y) {
      for (int x = 0; x != pal_width; ++x) {
        painter.drawPixmap(x * stride + border, y * stride + border, checkers);
      }
    }
  }
  
  gfx::Color getGColor(const PixelIndex idx) const {
    switch (format) {
      case Format::index:
      case Format::rgba:
        return FmtRgba::color(static_cast<PixelRgba>(palette[idx]));
      case Format::gray:
        return FmtGray::color(static_cast<PixelGray>(palette[idx]));
    }
  }
  
  bool useWhiteSelect() const {
    const gfx::Color gColor = getGColor(index);
    return gfx::gray(gColor) < 128 && gColor.a >= 128;
  }
  
  static QPoint entryPos(const PixelIndex idx) {
    return QPoint{idx % pal_width, idx / pal_width}
           * pal_tile_stride
           + toPoint(glob_border_width);
  }
  
  static int entryIdx(QPoint pos) {
    // Prevent clicking on borders
    // if (pos.x() % pal_tile_stride < glob_border_width) return -1;
    // if (pos.y() % pal_tile_stride < glob_border_width) return -1;
    pos = {pos.x() / pal_tile_stride, pos.y() / pal_tile_stride};
    if (pos.x() < 0 || pos.x() >= pal_width) return -1;
    if (pos.y() < 0 || pos.y() >= pal_height) return -1;
    return pos.y() * pal_width + pos.x();
  }
  
  void paintColors(QPainter &painter) {
    for (int idx = 0; idx != pal_colors; ++idx) {
      painter.setBrush(convert(getGColor(idx)));
      painter.drawRect(QRect{entryPos(idx), toSize(pal_tile_size)});
    }
  }
  
  void paintSelect(QPainter &painter) {
    if (selected) {
      painter.drawPixmap(entryPos(index), useWhiteSelect() ? selectWhite : selectBlack);
    }
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.setPen(Qt::NoPen);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawPixmap(rect(), background);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    paintColors(painter);
    paintSelect(painter);
  }
  
  void mousePressEvent(QMouseEvent *event) override {
    const int idx = entryIdx(event->pos());
    if (idx == -1) return;
    if (event->button() == Qt::LeftButton) {
      attachIndex(idx);
    } else if (event->button() == Qt::RightButton) {
      if (format == Format::index) {
        attachIndex(idx);
        Q_EMIT shouldSetIndex(index);
      } else {
        Q_EMIT shouldSetColor(palette[idx]);
      }
    }
  }
  
  void mouseMoveEvent(QMouseEvent *event) override {
    const int idx = entryIdx(event->pos());
    
    if (idx == -1) {
      unsetCursor();
      Q_EMIT shouldShowNorm("");
      return;
    }
    
    setCursor(Qt::PointingHandCursor);
    
    StatusMsg status;
    status.append("Index: ");
    status.append(idx);
    status.append(" Color: ");
    const gfx::Color gColor = getGColor(idx);
    if (format == Format::gray) {
      status.append(gColor.r, gColor.a);
    } else if (format == Format::rgba || format == Format::index) {
      status.append(gColor.r, gColor.g, gColor.b, gColor.a);
    }
    Q_EMIT shouldShowNorm(status.get());
  }
  
  void leaveEvent(QEvent *) override {
    Q_EMIT shouldShowNorm("");
  }
  
  PixelVar getInitialColor() const override {
    return palette[index];
  }
  void setColor(const PixelVar newColor) override {
    palette[index] = newColor;
    Q_EMIT paletteColorChanged();
    update();
  }
  QString getName() const override {
    return "Palette " + QString::number(index);
  }
  void detach() override {
    selected = false;
    update(QRect{entryPos(index), toSize(pal_tile_size)});
  }
};

PaletteWidget::PaletteWidget(QWidget *parent)
  : QScrollArea{parent} {
  setFrameShape(NoFrame);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  setFixedWidth(pick_svgraph_rect.widget().width() + 2 * glob_border_width);
  setMinimumHeight(pal_tile_stride + glob_border_width);
  setAlignment(Qt::AlignHCenter);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void PaletteWidget::initCanvas(const Format format) {
  table = new PaletteTableWidget{this, format};
  setWidget(table);
  CONNECT(table, shouldAttachColor,   this, shouldAttachColor);
  CONNECT(table, shouldSetColor,      this, shouldSetColor);
  CONNECT(table, shouldSetIndex,      this, shouldSetIndex);
  CONNECT(table, paletteColorChanged, this, paletteColorChanged);
  CONNECT(table, shouldShowNorm,      this, shouldShowNorm);
}

void PaletteWidget::setPalette(const PaletteSpan palette) {
  table->setPalette(palette);
}

void PaletteWidget::attachIndex(const PixelIndex index) {
  table->attachIndex(index);
}

void PaletteWidget::updatePalette() {
  table->updatePalette();
}

#include "palette widget.moc"
