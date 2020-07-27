//
//  json atlas generator.hpp
//  Animera
//
//  Created by Indiana Kernick on 27/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_json_atlas_generator_hpp
#define animera_json_atlas_generator_hpp

#include <unordered_set>
#include "sprite packer.hpp"
#include "atlas generator.hpp"
#include <QtCore/qjsondocument.h>

class JsonAtlasGenerator final : public AtlasGenerator {
public:
  JsonAtlasGenerator();

  Error initAtlas(PixelFormat, const QString &, const QString &) override;
  
  void addName(std::size_t, const SpriteNameParams &, const SpriteNameState &) override;
  void addSize(QSize) override;
  void addWhiteName() override;
  QString hasNameCollision() override;
  Error packRectangles() override;
  
  Error initAnimation(Format, PaletteCSpan) override;
  Error addImage(std::size_t, const QImage &) override;
  Error addWhiteImage() override;
  
  Error finalize() override;

private:
  SpritePacker packer;
  QString atlas;
  QString atlasDir;
  QString atlasName;
  std::unordered_set<QString> names;
  QString collision;
  
  void insertName(const QString &);
  void appendName(const QString &, std::size_t);
  void appendRect(QRect);
};

#endif
