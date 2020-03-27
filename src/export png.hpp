//
//  export png.hpp
//  Animera
//
//  Created by Indi Kernick on 20/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef export_png_hpp
#define export_png_hpp

#include "error.hpp"
#include "palette span.hpp"
#include "export options.hpp"

Error exportPng(QIODevice &, PaletteCSpan, QImage, Format, ExportFormat);
Error importPng(QIODevice &, PaletteSpan, QImage &, Format &);
Error exportPalettePng(QIODevice &, PaletteCSpan, Format);
Error importPalettePng(QIODevice &, PaletteSpan, Format);

#endif
