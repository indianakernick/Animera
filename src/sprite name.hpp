//
//  sprite name.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_sprite_name_hpp
#define animera_sprite_name_hpp

#include "cel.hpp"

enum class LayerNameMode {
  // export dialog.cpp and cli export.cpp depend on order
  automatic,
  name,
  index,
  empty
};

enum class GroupNameMode {
  // export dialog.cpp and cli export.cpp depend on order
  automatic,
  name,
  index,
  empty,
  sheet_column,
  sheet_row
};

enum class FrameNameMode {
  // export dialog.cpp and cli export.cpp depend on order
  automatic,
  relative,
  absolute,
  empty,
  sheet_column,
  sheet_row
};

struct SpriteNameParams {
  QString baseName;
  LayerNameMode layerName;
  GroupNameMode groupName;
  FrameNameMode frameName;
};

struct SpriteNameState {
  LayerIdx layer;
  GroupIdx group;
  FrameIdx frame;
  
  LayerIdx layerCount;
  GroupIdx groupCount;
  FrameIdx frameCount;
  
  FrameIdx maxGroupFrameCount;
  FrameIdx groupFrameCount;
  FrameIdx groupBegin;
  
  std::string_view layerName;
  std::string_view groupName;
};

void appendLayerName(QString &, const SpriteNameParams &, const SpriteNameState &);
void appendGroupName(QString &, const SpriteNameParams &, const SpriteNameState &);
void appendFrameName(QString &, const SpriteNameParams &, const SpriteNameState &);
QString evaluateSpriteName(const SpriteNameParams &, const SpriteNameState &);

QString nameFromPath(const QString &);
QString nameDirFromPath(QString &);

#endif
