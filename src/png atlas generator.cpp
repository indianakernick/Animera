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

bool PngAtlasGenerator::supported(const PixelFormat newPixelFormat, const Format newFormat) const {
  switch (newPixelFormat) {
    case PixelFormat::rgba:
      return newFormat == Format::rgba || newFormat == Format::index;
    case PixelFormat::index:
      return newFormat == Format::index;
    case PixelFormat::gray:
      return newFormat == Format::index || newFormat == Format::gray;
    case PixelFormat::gray_alpha:
      return newFormat == Format::gray;
    case PixelFormat::monochrome:
      return newFormat == Format::index || newFormat == Format::gray;
  }
}

Error PngAtlasGenerator::beginAtlas(const AtlasInfo &info) {
  pixelFormat = info.pixelFormat;
  directory = info.directory;
  return {};
}

void PngAtlasGenerator::appendName(std::size_t, const NameInfo info) {
  names.push_back(evaluateSpriteName(info.params, info.state));
}

void PngAtlasGenerator::appendWhiteName(std::size_t) {}

QString PngAtlasGenerator::endNames() {
  std::unordered_set<QString> set;
  for (const QString &name : names) {
    if (!set.insert(name).second) {
      return name;
    }
  }
  return {};
}

Error PngAtlasGenerator::beginImages() {
  return {};
}

Error PngAtlasGenerator::setImageFormat(const Format newFormat, const PaletteCSpan newPalette) {
  format = newFormat;
  palette = newPalette;
  return {};
}

Error PngAtlasGenerator::copyImage(const std::size_t i, const QImage &image) {
  FileWriter writer;
  TRY(writer.open(directory + QDir::separator() + names[i] + ".png"));
  TRY(exportCelPng(writer.dev(), palette, image, format, pixelFormat));
  return writer.flush();
}

Error PngAtlasGenerator::copyWhiteImage(std::size_t) {
  return {};
}

Error PngAtlasGenerator::endAtlas() {
  return {};
}
