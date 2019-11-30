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


// This is quite similar to SpriteInfo in sprite file.hpp
struct ExportSpriteInfo {
  LayerIdx layers;
  FrameIdx frames;
  Format format;
};

class Sprite;

ExportSpriteInfo getSpriteInfo(const Sprite &);

QString getExportPath(const ExportOptions &, ExportState);

// if we end up fixing up pattern then we should definitely tell the user
// "did you actually mean this?"
// That's a pretty good error message!

// Would Utils::ParseString be useful?
//   QString might make that annoying

void initDefaultOptions(ExportOptions &, ExportSpriteInfo);
ExportOptions exportFrameOptions(const QString &, ExportSpriteInfo);
Error readExportOptions(ExportOptions &, ExportSpriteInfo, const std::map<std::string, docopt::value> &);

#endif
