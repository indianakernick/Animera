//
//  png.cpp
//  Animera
//
//  Created by Indi Kernick on 4/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "png.hpp"

#include <iostream>
#include <QtCore/qfile.h>

QString pngErrorMsg = []{
  QString msg;
  msg.reserve(1024);
  return msg;
}();

void pngError(png_structp png, png_const_charp msg) {
  pngErrorMsg.append(msg);
  png_longjmp(png, 1);
}

void pngWarning(png_structp, png_const_charp msg) {
  std::cout << "PNG warning: " << msg << '\n';
}

void pngWrite(png_structp png, png_bytep data, size_t size) {
  auto *file = reinterpret_cast<QFile *>(png_get_io_ptr(png));
  const size_t written = file->write(reinterpret_cast<const char *>(data), size);
  if (written < size) {
    pngError(png, "Failed to write to output file");
  }
}

void pngRead(png_structp png, png_bytep data, size_t size) {
  auto *file = reinterpret_cast<QFile *>(png_get_io_ptr(png));
  const size_t read = file->read(reinterpret_cast<char *>(data), size);
  if (read < size) {
    pngError(png, "Truncated or invalid input file");
  }
}

void pngFlush(png_structp png) {
  auto *file = reinterpret_cast<QFile *>(png_get_io_ptr(png));
  if (!file->flush()) {
    pngError(png, "Failed to flush output file");
  }
}
