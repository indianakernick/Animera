//
//  png atlas generator.hpp
//  Animera
//
//  Created by Indiana Kernick on 16/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_png_atlas_generator_hpp
#define animera_png_atlas_generator_hpp

#include "atlas generator.hpp"

class PngAtlasGenerator final : public AtlasGenerator {
public:
  Error initAtlas(PixelFormat, const QString &, const QString &) override;
  
  void addName(std::size_t, const ExportNameParams &, const ExportNameState &) override;
  void addSizes(std::size_t, QSize) override;
  void addWhiteName() override;
  QString hasNameCollision() override;
  Error packRectangles() override;
  
  Error initAnimation(Format, PaletteCSpan) override;
  Error addImage(std::size_t, const QImage &) override;
  Error addWhiteImage() override;
  
  Error finalize() override;

private:
  PixelFormat pixelFormat;
  QString directory;
  Format format;
  PaletteCSpan palette;
  std::vector<QString> names;
};

#endif
