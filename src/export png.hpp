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
#include "export options.hpp"

/// Export a cell as a PNG
Error exportCellPng(QIODevice &, PaletteCSpan, QImage, Format, ExportFormat);
/// Import a cell as a PNG
Error importCellPng(QIODevice &, QImage &, Format);

/// Import a sprite as a PNG
Error importSpritePng(QIODevice &, PaletteSpan, QImage &, Format &);

/// Export the palette as a PNG
Error exportPalettePng(QIODevice &, PaletteCSpan, Format);
/// Import the palette as a PNG
Error importPalettePng(QIODevice &, PaletteSpan, Format);

#endif
