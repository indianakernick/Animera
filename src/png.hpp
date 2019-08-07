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

// might want to use Z_RLE

extern QString pngErrorMsg;

void pngError(png_structp, png_const_charp);
void pngWarning(png_structp, png_const_charp);
void pngWrite(png_structp, png_bytep, size_t);
void pngRead(png_structp, png_bytep, size_t);
void pngFlush(png_structp);

template <typename Elem, ptrdiff_t Size>
png_unknown_chunk makeUnknownChunk(
  const char (&name)[5],
  std::span<Elem, Size> span
) {
  png_unknown_chunk chunk;
  std::memcpy(chunk.name, name, 5);
  chunk.data = reinterpret_cast<png_byte *>(span.data());
  chunk.size = span.size_bytes();
  chunk.location = 0;
  return chunk;
}

template <typename Elem, ptrdiff_t Size>
void setUnknownChunk(
  png_structp png,
  png_infop info,
  const char (&name)[5],
  std::span<Elem, Size> span
) {
  png_unknown_chunk chunk = makeUnknownChunk(name, span);
  png_set_unknown_chunks(png, info, &chunk, 1);
}

#endif
