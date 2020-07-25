//
//  cpp atlas generator.hpp
//  Animera
//
//  Created by Indiana Kernick on 18/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_cpp_atlas_generator_hpp
#define animera_cpp_atlas_generator_hpp

#include <unordered_set>
#include "sprite packer.hpp"
#include "atlas generator.hpp"

class CppAtlasGenerator final : public AtlasGenerator {
public:
  explicit CppAtlasGenerator(DataFormat, bool = false);

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
  QString enumeration;
  QString array;
  std::unordered_set<QString> names;
  QString collision;
  QString atlasDir;
  QString atlasName;
  bool withInflate;
  
  void addAlias(QString, const char *, std::size_t);
  void appendEnumerator(const QString &, std::size_t);
  void appendRectangle(const QRect &);
  void insertName(const QString &);
  
  Error writeBytes(QIODevice &, const char *, std::size_t);
  Error writeCpp();
  Error writeHpp();
};

#endif
