//
//  png.cpp
//  Animera
//
//  Created by Indi Kernick on 4/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "png.hpp"

#include <iostream>
#include <QtCore/qstring.h>
#include <QtCore/qiodevice.h>

void pngError(png_structp png, png_const_charp msg) {
  reinterpret_cast<QString *>(png_get_error_ptr(png))->append(msg);
  png_longjmp(png, 1);
}

void pngWarning(png_structp, png_const_charp msg) {
  std::cout << "PNG warning: " << msg << '\n';
}

void pngWrite(png_structp png, png_bytep data, size_t size) {
  auto *dev = reinterpret_cast<QIODevice *>(png_get_io_ptr(png));
  const size_t written = dev->write(reinterpret_cast<const char *>(data), size);
  if (written < size) {
    pngError(png, "Failed to write to output file");
  }
}

void pngRead(png_structp png, png_bytep data, size_t size) {
  auto *dev = reinterpret_cast<QIODevice *>(png_get_io_ptr(png));
  const size_t read = dev->read(reinterpret_cast<char *>(data), size);
  if (read < size) {
    pngError(png, "Truncated or invalid input file");
  }
}

void pngFlush(png_structp) {}
