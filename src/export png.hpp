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

Error exportPng(const QString &, PaletteCSpan, QImage, Format, ExportFormat);

#endif
