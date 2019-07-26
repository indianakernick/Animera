//
//  export options.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 25/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef export_options_hpp
#define export_options_hpp

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
  indexed,
  grayscale,
  monochrome
};

struct Line {
  int stride;
  int offset;
};

struct ExportOptions {
  QString name;
  QString directory;
  Line layerLine;
  Line frameLine;
  LayerSelect layerSelect;
  FrameSelect frameSelect;
  ExportFormat format;
};

#endif
