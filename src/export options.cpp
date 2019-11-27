//
//  export options.cpp
//  Animera
//
//  Created by Indi Kernick on 24/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export options.hpp"

#include "strings.hpp"
#include <QtCore/qdir.h>
#include "export pattern.hpp"

namespace {

template <typename Idx>
Idx applyLine(const Line<Idx> line, const Idx value) {
  return value * line.stride + line.offset;
}

}

QString getExportPath(const ExportOptions &options, ExportState state) {
  QString path = options.directory;
  if (path.back() != QDir::separator()) {
    path.push_back(QDir::separator());
  }
  state.currPos.l = applyLine(options.layerLine, state.currPos.l);
  state.currPos.f = applyLine(options.frameLine, state.currPos.f);
  path += evalExportPattern(options.name, state);
  path += ".png";
  return path;
}

CellRect getExportRect(const ExportOptions &options, const ExportSpriteInfo &info) {
  CellRect rect;
  switch (options.layerSelect) {
    case LayerSelect::all_composited:
    case LayerSelect::all:
      rect.minL = LayerIdx{0};
      rect.maxL = info.layers - LayerIdx{1};
      break;
    case LayerSelect::current:
      rect.minL = rect.maxL = info.currPos.l;
      break;
    default: Q_UNREACHABLE();
  }
  switch (options.frameSelect) {
    case FrameSelect::all:
      rect.minF = FrameIdx{0};
      rect.maxF = info.frames - FrameIdx{1};
      break;
    case FrameSelect::current:
      rect.minF = rect.maxF = info.currPos.f;
      break;
    default: Q_UNREACHABLE();
  }
  return rect;
}

void initDefaultOptions(ExportOptions &options, const Format canvasFormat) {
  options.name = "sprite_%000F";
  options.directory = ".";
  options.layerLine = {LayerIdx{1}, LayerIdx{0}};
  options.frameLine = {FrameIdx{1}, FrameIdx{0}};
  options.layerSelect = LayerSelect::all_composited;
  options.frameSelect = FrameSelect::all;
  switch (canvasFormat) {
    case Format::rgba:
      options.format = ExportFormat::rgba;
      break;
    case Format::index:
      options.format = ExportFormat::rgba;
      break;
    case Format::gray:
      options.format = ExportFormat::gray_alpha;
      break;
  }
  options.scaleX = 1;
  options.scaleY = 1;
  options.angle = 0;
}

ExportOptions exportFrameOptions(const QString &path, const Format canvasFormat) {
  QString dir = path;
  dir.chop(dir.size() - dir.lastIndexOf('.'));
  const int lastSlash = dir.lastIndexOf(QDir::separator());
  const int nameLen = dir.size() - lastSlash - 1;
  QString name{dir.data() + lastSlash + 1, nameLen};
  dir.chop(nameLen);
  ExportOptions options;
  initDefaultOptions(options, canvasFormat);
  options.name = std::move(name);
  options.directory = std::move(dir);
  return options;
}

Error readExportOptions(
  ExportOptions &options,
  CellPos &current,
  const Format format,
  const std::map<std::string, docopt::value> &flags
) {
  if (const docopt::value &name = flags.at("--name"); name) {
    // TODO: ensure that name is a string
    // TODO: validate pattern
    options.name = toLatinString(name.asString());
  }
  if (const docopt::value &dir = flags.at("--directory"); dir) {
    // TODO: ensure that dir is a string
    // TODO: ensure this is a valid directory
    options.directory = toLatinString(dir.asString());
  }
  
  // TODO: check for unknown flags
  
  return {};
}
