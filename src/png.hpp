//
//  png.hpp
//  Animera
//
//  Created by Indiana Kernick on 4/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_png_hpp
#define animera_png_hpp

#include <cstddef>
#include <libpng16/png.h>

void pngError(png_structp, png_const_charp);
void pngWarning(png_structp, png_const_charp);
void pngWrite(png_structp, png_bytep, std::size_t);
void pngRead(png_structp, png_bytep, std::size_t);
void pngFlush(png_structp);

#endif
