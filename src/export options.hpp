//
//  export options.hpp
//  Animera
//
//  Created by Indi Kernick on 25/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef export_options_hpp
#define export_options_hpp

#include "cell.hpp"
#include "docopt.h"
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

template <typename Idx>
struct Line {
  Idx stride;
  Idx offset;
};

struct ExportOptions {
  QString name;
  QString directory;
  Line<LayerIdx> layerLine;
  Line<FrameIdx> frameLine;
  CellRect selection;
  ExportFormat format;
  ExportVis visibility;
  int scaleX;
  int scaleY;
  int angle;
  bool composite;
};

/// State used to evaluate sprite name pattern
struct ExportState {
  CellPos currPos;
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
ExportOptions exportCellOptions(const QString &, const ExportSpriteInfo &);
Error readExportOptions(
  ExportOptions &,
  const ExportSpriteInfo &,
  const std::map<std::string, docopt::value> &
);

#endif
