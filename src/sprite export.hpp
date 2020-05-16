//
//  sprite export.hpp
//  Animera
//
//  Created by Indiana Kernick on 5/10/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_sprite_export_hpp
#define animera_sprite_export_hpp

#include "error.hpp"
#include "cel span.hpp"
#include "palette span.hpp"
#include "export options.hpp"

class Exporter {
public:
  Exporter(const ExportOptions &, PaletteCSpan, Format, QSize);
  
  Error exportSprite(const std::vector<Layer> &);

private:
  const ExportOptions &options;
  PaletteCSpan palette;
  Format format;
  QImage image;
  QImage xformed;
  QSize size;
  
  void initImages();
  QSize getXformedSize() const;
  void setImageFrom(const Cel &);
  void setImageFrom(const Frame &);
  void applyTransform();
  Error exportImage(ExportState);
  bool shouldInclude(const Layer &) const;
  Error exportCels(const std::vector<Layer> &);
  Error exportFrames(const std::vector<Layer> &);
};

#endif
