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
#include <QtCore/qstring.h>

enum class LayerSelect {
  // export dialog.cpp depends on order
  all_composited,
  all,
  current
};

constexpr bool composited(const LayerSelect layer) {
  return layer == LayerSelect::all_composited;
}

enum class FrameSelect {
  // export dialog.cpp depends on order
  all,
  current
};

enum class ExportFormat {
  rgba,
  index,
  gray,
  gray_alpha,
  monochrome
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
  LayerSelect layerSelect;
  FrameSelect frameSelect;
  ExportFormat format;
  int scaleX;
  int scaleY;
  int angle;
};

#endif
