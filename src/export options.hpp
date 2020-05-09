//
//  export options.hpp
//  Animera
//
//  Created by Indiana Kernick on 25/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_export_options_hpp
#define animera_export_options_hpp

#include "cel.hpp"
#include <docopt.h>
#include "error.hpp"
#include <QtCore/qstring.h>

enum class ExportFormat {
  // export options.cpp and export dialog.cpp depend on order
  rgba,
  index,
  gray,
  gray_alpha,
  monochrome
};

enum class ExportVis {
  // export options.cpp depends on order
  visible,
  hidden,
  all
};

struct ExportOptions {
  QString name;
  QString directory;
  CelRect selection;
  ExportFormat format;
  ExportVis visibility;
  int scaleX;
  int scaleY;
  int angle;
  bool composite;
};

/// State used to evaluate sprite name pattern
struct ExportState {
  CelPos pos;
  // Could add layer name to this
};

struct ExportSpriteInfo {
  LayerIdx layers;
  FrameIdx frames;
  LayerIdx layer;
  FrameIdx frame;
  Format format;
};

class Sprite;

ExportSpriteInfo getSpriteInfo(const Sprite &);
QString getExportPath(const ExportOptions &, ExportState);
void initDefaultOptions(ExportOptions &, const ExportSpriteInfo &);
ExportOptions exportFrameOptions(const QString &, const ExportSpriteInfo &);
ExportOptions exportCelOptions(const QString &, const ExportSpriteInfo &);

#endif
