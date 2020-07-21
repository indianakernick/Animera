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
#include "export name.hpp"
#include "palette span.hpp"

enum class PixelFormat {
  // cli export.cpp depends on order
  rgba,
  index,
  gray,
  gray_alpha,
  monochrome
};

class AtlasGenerator {
public:
  virtual ~AtlasGenerator() = default;
  
  virtual Error initAtlas(PixelFormat, const QString &, const QString &) = 0;
  
  // First pass
  // Add the names and rectangles
  virtual void addWhiteName() = 0;
  virtual void addName(std::size_t, const ExportNameParams &, const ExportNameState &) = 0;
  virtual void addSizes(std::size_t, QSize) = 0;
  virtual QString hasNameCollision() = 0;
  virtual Error packRectangles() = 0;
  
  // Second pass
  // Add the images
  virtual Error initAnimation(Format, PaletteCSpan) = 0;
  virtual Error addWhiteImage() = 0;
  virtual Error addImage(std::size_t, const QImage &) = 0;
  
  virtual Error finalize() = 0;
};

#endif
