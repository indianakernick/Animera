//
//  palette.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef palette_hpp
#define palette_hpp

#include <array>
#include "config.hpp"
#include <QtGui/qrgb.h>
#include <QtCore/qobject.h>

class QIODevice;

using PaletteColors = std::array<QRgb, pal_colors>;

// @TODO use std::span when available

class PaletteSpan {
public:
  PaletteSpan() = default;
 
  QRgb &operator[](const uint8_t idx) const {
    assert(colors);
    return colors[idx];
  }
  QRgb *data() const {
    return colors;
  }
  
private:
  QRgb *colors;
  
  friend class Palette;
  explicit PaletteSpan(QRgb *colors)
    : colors{colors} {}
};

class PaletteCSpan {
public:
  PaletteCSpan() = default;
  PaletteCSpan(const PaletteSpan span)
    : colors{span.data()} {}

  QRgb operator[](const uint8_t idx) const {
    assert(colors);
    return colors[idx];
  }
  const QRgb *data() const {
    return colors;
  }

private:
  const QRgb *colors;
  
  friend class Palette;
  explicit PaletteCSpan(QRgb *colors)
    : colors{colors} {}
};

class Palette final : public QObject {
  Q_OBJECT

public:
  void initDefault();
  void serialize(QIODevice *) const;
  void deserialize(QIODevice *);

public Q_SLOTS:
  void initCanvas();

Q_SIGNALS:
  void paletteChanged(PaletteSpan);
  
private:
  PaletteColors colors;
};

#endif
