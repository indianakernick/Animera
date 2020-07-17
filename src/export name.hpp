//
//  export name.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_export_name_hpp
#define animera_export_name_hpp

#include "cel.hpp"

enum class LayerNameMode {
  // export dialog.cpp depends on order
  automatic,
  name,
  index,
  empty
};

enum class GroupNameMode {
  // export dialog.cpp depends on order
  automatic,
  name,
  index,
  empty
};

enum class FrameNameMode {
  // export dialog.cpp depends on order
  automatic,
  relative,
  absolute,
  empty
};

struct ExportNameParams {
  QString name;
  LayerNameMode layerName;
  GroupNameMode groupName;
  FrameNameMode frameName;
};

struct ExportNameState {
  LayerIdx layer;
  GroupIdx group;
  FrameIdx frame;
  
  LayerIdx layerCount;
  GroupIdx groupCount;
  FrameIdx frameCount;
  
  FrameIdx groupBegin;
  
  std::string_view layerName;
  std::string_view groupName;
};

#endif
