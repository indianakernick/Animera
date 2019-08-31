//
//  png.hpp
//  Animera
//
//  Created by Indi Kernick on 4/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef png_hpp
#define png_hpp

#include <span>
#include <libpng16/png.h>
#include <QtCore/qstring.h>

extern QString pngErrorMsg;

void pngError(png_structp, png_const_charp);
void pngWarning(png_structp, png_const_charp);
void pngWrite(png_structp, png_bytep, size_t);
void pngRead(png_structp, png_bytep, size_t);
void pngFlush(png_structp);

#endif
