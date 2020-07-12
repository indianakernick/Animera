//
//  sprite file.hpp
//  Animera
//
//  Created by Indiana Kernick on 31/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_sprite_file_hpp
#define animera_sprite_file_hpp

#include "cel.hpp"
#include "error.hpp"
#include "image.hpp"
#include "group array.hpp"
#include "palette span.hpp"

class QIODevice;
struct Layer;
struct CelSpan;

struct SpriteInfo {
  int width;
  int height;
  LayerIdx layers;
  FrameIdx frames;
  int delay;
  Format format;
};

Error writeSignature(QIODevice &);
Error writeAHDR(QIODevice &, const SpriteInfo &);
Error writePLTE(QIODevice &, PaletteCSpan, Format);
Error writeGRUP(QIODevice &, const std::vector<Group> &);
Error writeLHDR(QIODevice &, const Layer &);
Error writeCHDR(QIODevice &, const CelSpan &);
Error writeCDAT(QIODevice &, const QImage &, Format);
Error writeAEND(QIODevice &);

Error readSignature(QIODevice &);
Error readAHDR(QIODevice &, SpriteInfo &);
Error readPLTE(QIODevice &, PaletteSpan, Format);
Error readGRUP(QIODevice &, std::vector<Group> &);
Error readLHDR(QIODevice &, Layer &);
Error readCHDR(QIODevice &, CelSpan &, Format);
Error readCDAT(QIODevice &, QImage &, Format);
Error readAEND(QIODevice &);

#endif
