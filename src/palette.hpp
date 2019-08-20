//
//  palette.hpp
//  Animera
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef palette_hpp
#define palette_hpp

#include <span>
#include <array>
#include "error.hpp"
#include "image.hpp"
#include "config.hpp"
#include <QtGui/qrgb.h>
#include <QtCore/qobject.h>

using PaletteColors = std::array<QRgb, pal_colors>;
using PaletteSpan = std::span<QRgb>;
using PaletteCSpan = std::span<const QRgb>;

class Palette final : public QObject {
  Q_OBJECT

public:
  void initDefault();
  Error serialize(QIODevice &) const;
  Error deserialize(QIODevice &);

  PaletteCSpan getPalette() const;

public Q_SLOTS:
  void initCanvas(Format);

Q_SIGNALS:
  void paletteChanged(PaletteSpan);
  
private:
  PaletteColors colors;
  Format canvasFormat;
};

#endif
