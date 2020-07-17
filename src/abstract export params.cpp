//
//  abstract export params.cpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "abstract export params.hpp"

#include "sprite.hpp"
#include <QtCore/qdir.h>
#include "png export backend.hpp"

ExportSpriteInfo getSpriteInfo(const Sprite &sprite) {
  return {
    sprite.timeline.getPos(),
    sprite.timeline.getSelection(),
    sprite.timeline.getLayers(),
    sprite.timeline.getFrames(),
    sprite.getFormat()
  };
}

LayerRange selectLayers(const ExportSpriteInfo &info, const LayerSelection select) {
  switch (select) {
    case LayerSelection::visible:
      return {LayerIdx{0}, info.layerCount - LayerIdx{1}, LayerVis::visible};
    case LayerSelection::hidden:
      return {LayerIdx{0}, info.layerCount - LayerIdx{1}, LayerVis::hidden};
    case LayerSelection::all:
      return {LayerIdx{0}, info.layerCount - LayerIdx{1}, LayerVis::all};
    case LayerSelection::current:
      return {info.pos.l, info.pos.l, LayerVis::all};
    case LayerSelection::selected:
      return {info.selection.minL, info.selection.maxL, LayerVis::all};
  }
}

FrameRange selectFrames(const ExportSpriteInfo &info, const FrameSelection select) {
  switch (select) {
    case FrameSelection::all:
      return {FrameIdx{0}, info.frameCount - FrameIdx{1}};
    case FrameSelection::current:
      return {info.pos.f, info.pos.f};
    case FrameSelection::selected:
      return {info.selection.minF, info.selection.maxF};
  }
}

FrameRange selectFrames(const tcb::span<const Group> groups, const GroupIdx group) {
  const GroupInfo info = getGroup(groups, group);
  return {info.begin, info.begin + info.end - FrameIdx{1}};
}

namespace {

void setFormat(ExportParams &params, const ExportSpriteInfo &info) {
  switch (info.format) {
    case Format::rgba:
      params.pixelFormat = PixelFormat::rgba;
      break;
    case Format::index:
      params.pixelFormat = PixelFormat::rgba;
      break;
    case Format::gray:
      params.pixelFormat = PixelFormat::gray_alpha;
      break;
  }
}

void setDefaultTransform(ExportTransform &transform) {
  transform.scaleX = transform.scaleY = 1;
  transform.angle = 0;
}

void setPath(ExportParams &params, const QString &path) {
  QString dir = path;
  dir.chop(dir.size() - dir.lastIndexOf('.'));
  const int lastSlash = dir.lastIndexOf(QDir::separator());
  const int nameLen = dir.size() - lastSlash - 1;
  QString name{dir.data() + lastSlash + 1, nameLen};
  dir.chop(nameLen);
  params.name = "";
  params.directory = std::move(dir);
  params.sprites[0].name.name = std::move(name);
  params.sprites[0].name.layerName = LayerNameMode::empty;
  params.sprites[0].name.groupName = GroupNameMode::empty;
  params.sprites[0].name.frameName = FrameNameMode::empty;
}

}

ExportParams exportFrameParams(const ExportSpriteInfo &info, const QString &path) {
  ExportParams params;
  params.sprites.emplace_back();
  setPath(params, path);
  setDefaultTransform(params.sprites[0].transform);
  params.sprites[0].layers = {LayerIdx{}, info.layerCount - LayerIdx{1}, LayerVis::visible};
  params.sprites[0].frames = {info.pos.f, info.pos.f};
  params.sprites[0].composite = true;
  setFormat(params, info);
  params.backend = std::make_unique<PngExportBackend>();
  params.whitepixel = false;
  return params;
}

ExportParams exportCelParams(const ExportSpriteInfo &info, const QString &path) {
  ExportParams params;
  params.sprites.emplace_back();
  setPath(params, path);
  setDefaultTransform(params.sprites[0].transform);
  params.sprites[0].layers = {info.pos.l, info.pos.l, LayerVis::all};
  params.sprites[0].frames = {info.pos.f, info.pos.f};
  params.sprites[0].composite = false;
  setFormat(params, info);
  params.backend = std::make_unique<PngExportBackend>();
  params.whitepixel = false;
  return params;
}
