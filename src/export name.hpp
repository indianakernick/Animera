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
  empty
};

enum class FrameNameMode {
  // export dialog.cpp and cli export.cpp depend on order
  automatic,
  relative,
  absolute,
  empty
};

struct ExportNameParams {
  QString baseName;
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

void appendLayerName(QString &, const ExportNameParams &, const ExportNameState &);
void appendGroupName(QString &, const ExportNameParams &, const ExportNameState &);
void appendFrameName(QString &, const ExportNameParams &, const ExportNameState &);
QString evaluateExportName(const ExportNameParams &, const ExportNameState &);

QString nameFromPath(const QString &);
QString nameDirFromPath(QString &);

#endif
