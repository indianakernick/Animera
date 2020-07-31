//
//  atlas generator.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_atlas_generator_hpp
#define animera_atlas_generator_hpp

#include "error.hpp"
#include "sprite name.hpp"
#include "palette span.hpp"

struct AtlasInfo {
  QString name;
  QString directory;
  PixelFormat pixelFormat;
};

struct NameInfo {
  const SpriteNameParams &params;
  const SpriteNameState &state;
  QSize size;
};

class AtlasGenerator {
public:
  virtual ~AtlasGenerator() = default;
  
  /// Determines whether this generator can convert from the canvas format to
  /// the pixel format.
  virtual bool supported(PixelFormat, Format) const = 0;
  
  /// Start a new atlas
  virtual Error beginAtlas(const AtlasInfo &) = 0;
  
  /// Evaluate and append a new name to the atlas
  virtual void appendName(std::size_t, NameInfo) = 0;
  /// Append the white name to the atlas
  virtual void appendWhiteName(std::size_t) = 0;
  
  /// Finalize the names. Returns the first name to collide or null
  virtual QString endNames() = 0;
  /// Prepare to copy images
  virtual Error beginImages() = 0;
  
  /// Set the image format for future images passed to copyImage
  virtual Error setImageFormat(Format, PaletteCSpan) = 0;
  /// Copy the image to the texture
  virtual Error copyImage(std::size_t, const QImage &) = 0;
  /// Copy the white image to the texture
  virtual Error copyWhiteImage(std::size_t) = 0;
  
  /// Complete the atlas
  virtual Error endAtlas() = 0;
};

#endif
