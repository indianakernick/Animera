//
//  export options.cpp
//  Animera
//
//  Created by Indiana Kernick on 24/11/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "export options.hpp"

#include "sprite.hpp"
#include "strings.hpp"
#include <QtCore/qdir.h>
#include "export pattern.hpp"

ExportSpriteInfo getSpriteInfo(const Sprite &sprite) {
  return {
    sprite.timeline.getLayers(),
    sprite.timeline.getFrames(),
    sprite.timeline.getPos().l,
    sprite.timeline.getPos().f,
    sprite.getFormat()
  };
}

QString getExportPath(const ExportOptions &options, ExportState state) {
  QString path = options.directory;
  if (path.back() != QDir::separator()) {
    path.push_back(QDir::separator());
  }
  path += evalExportPattern(options.name, state);
  path += ".png";
  return path;
}

namespace {

void setFullSelection(ExportOptions &options, const ExportSpriteInfo &info) {
  options.selection.minL = LayerIdx{};
  options.selection.minF = FrameIdx{};
  options.selection.maxL = info.layers - LayerIdx{1};
  options.selection.maxF = info.frames - FrameIdx{1};
}

void setFormat(ExportOptions &options, const ExportSpriteInfo &info) {
  switch (info.format) {
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
}

void setDefaultTransform(ExportOptions &options) {
  options.scaleX = options.scaleY = 1;
  options.angle = 0;
}

void setPath(ExportOptions &options, const QString &path) {
  QString dir = path;
  dir.chop(dir.size() - dir.lastIndexOf('.'));
  const int lastSlash = dir.lastIndexOf(QDir::separator());
  const int nameLen = dir.size() - lastSlash - 1;
  QString name{dir.data() + lastSlash + 1, nameLen};
  dir.chop(nameLen);
  options.name = std::move(name);
  options.directory = std::move(dir);
}

}

void initDefaultOptions(ExportOptions &options, const ExportSpriteInfo &info) {
  options.name = "sprite_%000F";
  options.directory = ".";
  setFullSelection(options, info);
  setFormat(options, info);
  options.visibility = ExportVis::visible;
  setDefaultTransform(options);
  options.composite = true;
}

ExportOptions exportFrameOptions(const QString &path, const ExportSpriteInfo &info) {
  ExportOptions options;
  setPath(options, path);
  options.selection.minL = LayerIdx{};
  options.selection.minF = info.frame;
  options.selection.maxL = info.layers - LayerIdx{1};
  options.selection.maxF = info.frame;
  setFormat(options, info);
  options.visibility = ExportVis::visible;
  setDefaultTransform(options);
  options.composite = true;
  return options;
}

ExportOptions exportCelOptions(const QString &path, const ExportSpriteInfo &info) {
  ExportOptions options;
  setPath(options, path);
  options.selection.minL = info.layer;
  options.selection.minF = info.frame;
  options.selection.maxL = info.layer;
  options.selection.maxF = info.frame;
  setFormat(options, info);
  options.visibility = ExportVis::all;
  setDefaultTransform(options);
  options.composite = false;
  return options;
}
