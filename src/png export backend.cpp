//
//  png export backend.cpp
//  Animera
//
//  Created by Indiana Kernick on 16/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "png export backend.hpp"

#include "file io.hpp"
#include "strings.hpp"
#include <QtCore/qdir.h>
#include <unordered_set>
#include "export png.hpp"

namespace {

void appendSep(QString &str) {
  if (!str.isEmpty()) str.append(' ');
}

QString evaluateExportName(const ExportNameParams &params, const ExportNameState &state) {
  QString name = params.name;
  
  switch (params.layerName) {
    case LayerNameMode::automatic:
      if (state.layerCount <= LayerIdx{1}) break;
    case LayerNameMode::name:
      appendSep(name);
      name += toLatinString(state.layerName);
      break;
    case LayerNameMode::index:
      appendSep(name);
      name += QString::number(+state.layer);
      break;
    case LayerNameMode::empty:
      break;
  }
  
  switch (params.groupName) {
    case GroupNameMode::automatic:
      if (state.groupCount <= GroupIdx{1}) break;
    case GroupNameMode::name:
      appendSep(name);
      name += toLatinString(state.groupName);
      break;
    case GroupNameMode::index:
      appendSep(name);
      name += QString::number(+state.group);
      break;
    case GroupNameMode::empty:
      break;
  }
  
  switch (params.frameName) {
    case FrameNameMode::automatic:
      if (state.frameCount <= FrameIdx{1}) break;
    case FrameNameMode::relative:
      appendSep(name);
      name += QString::number(+(state.frame - state.groupBegin));
      break;
    case FrameNameMode::absolute:
      appendSep(name);
      name += QString::number(+state.frame);
      break;
    case FrameNameMode::empty:
      break;
  }
  
  return name;
}

}

Error PngExportBackend::initAtlas(
  const PixelFormat newPixelFormat,
  const QString &,
  const QString &newDirectory
) {
  pixelFormat = newPixelFormat;
  directory = newDirectory;
  return {};
}

void PngExportBackend::addWhiteName() {}

void PngExportBackend::addName(
  [[maybe_unused]] const std::size_t idx,
  const ExportNameParams &params,
  const ExportNameState &state
) {
  assert(idx == names.size());
  names.push_back(evaluateExportName(params, state));
}

void PngExportBackend::addSizes(std::size_t, QSize) {}

QString PngExportBackend::hasNameCollision() {
  // TODO: This seems inefficient. I'm not sure how to do it better.
  std::unordered_set<QString> set;
  for (const QString &name : names) {
    if (!set.insert(name).second) {
      return name;
    }
  }
  return {};
}

Error PngExportBackend::initAnimation(Format newFormat, PaletteCSpan newPalette) {
  format = newFormat;
  palette = newPalette;
  return {};
}

Error PngExportBackend::addWhiteImage() {
  return {};
}

Error PngExportBackend::addImage(const std::size_t idx, QImage image) {
  FileWriter writer;
  TRY(writer.open(directory + QDir::separator() + names[idx] + ".png"));
  TRY(exportCelPng(writer.dev(), palette, image, format, pixelFormat));
  return writer.flush();
}

Error PngExportBackend::finalize() {
  return {};
}
