//
//  sprite export.hpp
//  Animera
//
//  Created by Indi Kernick on 5/10/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef sprite_export_hpp
#define sprite_export_hpp

#include "error.hpp"
#include "cell span.hpp"
#include "palette span.hpp"
#include "export options.hpp"

class Exporter {
public:
  Exporter(const ExportOptions &, PaletteCSpan, Format, QSize);
  
  void setRect(LayerIdx, FrameIdx, CellPos);
  Error exportSprite(const std::vector<Layer> &);

private:
  const ExportOptions &options;
  PaletteCSpan palette;
  Format format;
  QImage image;
  CellRect rect;
  
  void setImageFrom(const Cell &);
  void setImageFrom(const Frame &);
  QString getPath(CellPos);
  Error exportImage(CellPos);
  Error exportCells(const std::vector<Layer> &);
  Error exportFrames(const std::vector<Layer> &);
};

#endif