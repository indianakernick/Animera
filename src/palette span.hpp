//
//  palette span.hpp
//  Animera
//
//  Created by Indiana Kernick on 31/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_palette_span_hpp
#define animera_palette_span_hpp

#include <array>
#include "config.hpp"
#include <tcb/span.hpp>

using PaletteColors = std::array<QRgb, pal_colors>;
using PaletteSpan = tcb::span<QRgb>;
using PaletteCSpan = tcb::span<const QRgb>;

#endif
