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
  bool supported(PixelFormat, Format) const override;

  Error beginAtlas(const AtlasInfo &) override;
  
  void appendName(std::size_t, NameInfo) override;
  void appendWhiteName(std::size_t) override;
  
  QString endNames() override;
  Error beginImages() override;
  
  Error setImageFormat(Format, PaletteCSpan) override;
  Error copyImage(std::size_t, const QImage &) override;
  Error copyWhiteImage(std::size_t) override;
  
  Error endAtlas() override;

private:
  PixelFormat pixelFormat;
  QString directory;
  Format format;
  PaletteCSpan palette;
  std::vector<QString> names;
};

#endif
