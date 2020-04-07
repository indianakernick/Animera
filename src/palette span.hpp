//
//  palette span.hpp
//  Animera
//
//  Created by Indiana Kernick on 31/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_palette_span_hpp
#define animera_palette_span_hpp

#include <span>
#include <array>
#include "config.hpp"

using PaletteColors = std::array<QRgb, pal_colors>;
using PaletteSpan = std::span<QRgb>;
using PaletteCSpan = std::span<const QRgb>;

#endif
