//
//  basic atlas generator.hpp
//  Animera
//
//  Created by Indiana Kernick on 28/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_basic_atlas_generator_hpp
#define animera_basic_atlas_generator_hpp

#include <unordered_set>
#include "sprite packer.hpp"
#include "atlas generator.hpp"

class BasicAtlasGenerator : public AtlasGenerator {
public:
  explicit BasicAtlasGenerator(DataFormat);

  Error initAtlas(PixelFormat, const QString &, const QString &) override;
  
  void addName(std::size_t, const SpriteNameParams &, const SpriteNameState &) override;
  void addSize(QSize) override;
  void addWhiteName() override;
  QString hasNameCollision() override;
  Error packRectangles() override;
  
  Error initAnimation(Format, PaletteCSpan) override;
  Error addImage(std::size_t, const QImage &) override;
  Error addWhiteImage() override;

  virtual void appendName(const QString &, std::size_t) = 0;
  virtual void appendRect(QRect) = 0;
  virtual void fixName(QString &, std::array<int, 4> &) = 0;
  virtual void appendAlias(QString, const char *, std::size_t) = 0;

protected:
  SpritePacker packer;

  void insertName(const QString &);

private:
  std::unordered_set<QString> names;
  QString collision;
};

#endif
