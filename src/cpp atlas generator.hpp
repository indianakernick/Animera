//
//  cpp atlas generator.hpp
//  Animera
//
//  Created by Indiana Kernick on 18/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_cpp_atlas_generator_hpp
#define animera_cpp_atlas_generator_hpp

#include "sprite packer.hpp"
#include "atlas generator.hpp"
#include "basic atlas generator.hpp"

class CppAtlasGenerator final : public BasicAtlasGenerator {
public:
  explicit CppAtlasGenerator(DataFormat, bool = false);

  Error beginAtlas(const AtlasInfo &) override;
  QString endNames() override;
  Error endAtlas() override;
  
  void appendName(const QString &, std::size_t) override;
  void appendRect(QRect) override;
  void fixName(QString &, std::array<int, 4> &) override;
  void appendAlias(QString, const char *, std::size_t) override;

private:
  QString enumeration;
  QString array;
  QString atlasName;
  QString atlasDir;
  bool withInflate;
  
  Error writeBytes(QIODevice &, const char *, std::size_t);
  Error writeCpp();
  Error writeHpp();
};

#endif
