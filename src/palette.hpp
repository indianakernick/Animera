//
//  palette.hpp
//  Animera
//
//  Created by Indiana Kernick on 7/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef palette_hpp
#define palette_hpp

#include "error.hpp"
#include "image.hpp"
#include <QtGui/qrgb.h>
#include "palette span.hpp"
#include <QtCore/qobject.h>

class Palette final : public QObject {
  Q_OBJECT

public:
  void reset();
  void initDefault();
  Error serialize(QIODevice &) const;
  Error deserialize(QIODevice &);
  Error save(const QString &) const;
  Error open(const QString &);

  PaletteSpan getPalette();
  PaletteCSpan getPalette() const;

  void change();

public Q_SLOTS:
  void initCanvas(Format);

Q_SIGNALS:
  void paletteChanged(PaletteSpan);
  
private:
  PaletteColors colors;
  Format canvasFormat;
};

#endif
