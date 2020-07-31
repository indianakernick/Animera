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

  bool supported(PixelFormat, Format) const override;

  Error beginAtlas(const AtlasInfo &) override;
  
  void appendName(std::size_t, NameInfo) override;
  void appendWhiteName(std::size_t) override;
  
  QString endNames() override;
  Error beginImages() override;
  
  Error setImageFormat(Format, PaletteCSpan) override;
  Error copyImage(std::size_t, const QImage &) override;
  Error copyWhiteImage(std::size_t) override;

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
