//
//  sprite name.cpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "sprite name.hpp"

#include "strings.hpp"

namespace {

void appendSep(QString &str) {
  if (!str.isEmpty()) str.append(' ');
}

void appendName(QString &name, const std::string_view str) {
  appendSep(name);
  name += toLatinString(str);
}

void appendIndex(QString &name, const int idx) {
  appendSep(name);
  name += QString::number(idx);
}

}

void appendLayerName(QString &name, const SpriteNameParams &params, const SpriteNameState &state) {
  switch (params.layerName) {
    case LayerNameMode::automatic:
      if (state.layerCount <= LayerIdx{1}) return;
      if (state.layerName.empty()) {
        return appendIndex(name, +state.layer);
      } else {
        return appendName(name, state.layerName);
      }
    case LayerNameMode::name:
      return appendName(name, state.layerName);
    case LayerNameMode::index:
      return appendIndex(name, +state.layer);
    case LayerNameMode::empty:
      return;
  }
}

void appendGroupName(QString &name, const SpriteNameParams &params, const SpriteNameState &state) {
  switch (params.groupName) {
    case GroupNameMode::automatic:
      if (state.groupCount <= GroupIdx{1}) return;
      if (state.groupName.empty()) {
        return appendIndex(name, +state.group);
      } else {
        return appendName(name, state.groupName);
      }
    case GroupNameMode::name:
      return appendName(name, state.groupName);
    case GroupNameMode::index:
      return appendIndex(name, +state.group);
    case GroupNameMode::empty:
    case GroupNameMode::sheet_column:
    case GroupNameMode::sheet_row:
      return;
  }
}

void appendFrameName(QString &name, const SpriteNameParams &params, const SpriteNameState &state) {
  switch (params.frameName) {
    case FrameNameMode::automatic:
      if (state.groupFrameCount <= FrameIdx{1}) return;
    case FrameNameMode::relative:
      return appendIndex(name, +(state.frame - state.groupBegin));
    case FrameNameMode::absolute:
      return appendIndex(name, +state.frame);
    case FrameNameMode::empty:
    case FrameNameMode::sheet_column:
    case FrameNameMode::sheet_row:
      return;
  }
}

QString evaluateSpriteName(const SpriteNameParams &params, const SpriteNameState &state) {
  QString name = params.baseName;
  appendLayerName(name, params, state);
  appendGroupName(name, params, state);
  appendFrameName(name, params, state);
  return name;
}

QString nameFromPath(const QString &path) {
  QString dir = path;
  return nameDirFromPath(dir);
}

QString nameDirFromPath(QString &path) {
  int begin = path.lastIndexOf('/');
  int end = path.lastIndexOf('.');
  begin += 1;
  end = end == -1 ? path.size() : end;
  QString name{path.data() + begin, end - begin};
  path.truncate(begin);
  return name;
}
