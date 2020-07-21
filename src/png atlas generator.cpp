//
//  png atlas generator.cpp
//  Animera
//
//  Created by Indiana Kernick on 16/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "png atlas generator.hpp"

#include "file io.hpp"
#include <QtCore/qdir.h>
#include <unordered_set>
#include "export png.hpp"

Error PngAtlasGenerator::initAtlas(
  const PixelFormat newPixelFormat,
  const QString &,
  const QString &newDirectory
) {
  pixelFormat = newPixelFormat;
  directory = newDirectory;
  return {};
}

void PngAtlasGenerator::addName(
  [[maybe_unused]] const std::size_t idx,
  const ExportNameParams &params,
  const ExportNameState &state
) {
  assert(idx == names.size());
  names.push_back(evaluateExportName(params, state));
}

void PngAtlasGenerator::addSizes(std::size_t, QSize) {}

void PngAtlasGenerator::addWhiteName() {}

QString PngAtlasGenerator::hasNameCollision() {
  // TODO: This seems inefficient. I'm not sure how to do it better.
  std::unordered_set<QString> set;
  for (const QString &name : names) {
    if (!set.insert(name).second) {
      return name;
    }
  }
  return {};
}

Error PngAtlasGenerator::packRectangles() {
  return {};
}

Error PngAtlasGenerator::initAnimation(Format newFormat, PaletteCSpan newPalette) {
  format = newFormat;
  palette = newPalette;
  return {};
}

Error PngAtlasGenerator::addImage(const std::size_t idx, const QImage &image) {
  FileWriter writer;
  TRY(writer.open(directory + QDir::separator() + names[idx] + ".png"));
  TRY(exportCelPng(writer.dev(), palette, image, format, pixelFormat));
  return writer.flush();
}

Error PngAtlasGenerator::addWhiteImage() {
  return {};
}

Error PngAtlasGenerator::finalize() {
  return {};
}
