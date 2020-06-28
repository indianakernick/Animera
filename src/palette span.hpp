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
#include "image.hpp"
#include <tcb/span.hpp>
#include "config geometry.hpp"

using PaletteColors = std::array<PixelVar, pal_colors>;
using PaletteSpan = tcb::span<PixelVar>;
using PaletteCSpan = tcb::span<const PixelVar>;

#endif
