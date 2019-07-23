//
//  palette.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef palette_hpp
#define palette_hpp

#include <span>
#include <array>
#include "image.hpp"
#include "config.hpp"
#include <QtGui/qrgb.h>
#include <QtCore/qobject.h>

class QIODevice;

using PaletteColors = std::array<QRgb, pal_colors>;
using PaletteSpan = std::span<QRgb>;
using PaletteCSpan = std::span<const QRgb>;

class Palette final : public QObject {
  Q_OBJECT

public:
  void initDefault();
  void serialize(QIODevice *) const;
  void deserialize(QIODevice *);

public Q_SLOTS:
  void initCanvas(Format);

Q_SIGNALS:
  void paletteChanged(PaletteSpan);
  
private:
  PaletteColors colors;
  Format canvasFormat;
};

#endif
