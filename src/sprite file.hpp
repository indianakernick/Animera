//
//  sprite file.hpp
//  Animera
//
//  Created by Indi Kernick on 31/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef sprite_file_hpp
#define sprite_file_hpp

#include "cell.hpp"
#include "error.hpp"
#include "image.hpp"
#include "palette span.hpp"

class QIODevice;
struct Layer;
struct CellSpan;

struct SpriteInfo {
  int width;
  int height;
  LayerIdx layers;
  FrameIdx frames;
  int delay;
  Format format;
};

Error writeAHDR(QIODevice &, const SpriteInfo &);
Error writePLTE(QIODevice &, PaletteCSpan, Format);
Error writeLHDR(QIODevice &, const Layer &);
Error writeCHDR(QIODevice &, const CellSpan &);
Error writeCDAT(QIODevice &, const QImage &, Format);
Error writeAEND(QIODevice &);

Error readAHDR(QIODevice &, SpriteInfo &);
Error readPLTE(QIODevice &, PaletteSpan, Format);
Error readLHDR(QIODevice &, Layer &);
Error readCHDR(QIODevice &, CellSpan &, Format);
Error readCDAT(QIODevice &, QImage &, Format);
Error readAEND(QIODevice &);

#endif
