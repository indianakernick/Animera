//
//  export png.hpp
//  Animera
//
//  Created by Indiana Kernick on 20/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_export_png_hpp
#define animera_export_png_hpp

#include "error.hpp"
#include "palette span.hpp"
#include "export params.hpp"

/// Export a cel as a PNG
Error exportCelPng(QIODevice &, PaletteCSpan, QImage, Format, PixelFormat);
/// Import a cel as a PNG
Error importCelPng(QIODevice &, QImage &, Format);

/// Import a animation as a PNG
Error importAnimationPng(QIODevice &, PaletteSpan, QImage &, Format &);

/// Export the palette as a PNG
Error exportPalettePng(QIODevice &, PaletteCSpan, Format);
/// Import the palette as a PNG
Error importPalettePng(QIODevice &, PaletteSpan, Format);

#endif
