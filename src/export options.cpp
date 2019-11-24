//
//  export options.cpp
//  Animera
//
//  Created by Indi Kernick on 24/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export options.hpp"

#include <QtCore/qdir.h>

ExportOptions exportFrameOptions(const QString &path, const Format canvasFormat) {
  ExportFormat format;
  switch (canvasFormat) {
    case Format::rgba:
      format = ExportFormat::rgba;
      break;
    case Format::index:
      format = ExportFormat::rgba;
      break;
    case Format::gray:
      format = ExportFormat::gray_alpha;
      break;
  }
  QString dir = path;
  dir.chop(dir.size() - dir.lastIndexOf('.'));
  const int lastSlash = dir.lastIndexOf(QDir::separator());
  const int nameLen = dir.size() - lastSlash - 1;
  QString name{dir.data() + lastSlash + 1, nameLen};
  dir.chop(nameLen);
  return {
    std::move(name),
    std::move(dir),
    {LayerIdx{1}, LayerIdx{0}},
    {FrameIdx{1}, FrameIdx{0}},
    LayerSelect::all_composited,
    FrameSelect::current,
    format,
    1,
    1,
    0
  };
}
