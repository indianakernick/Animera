//
//  json atlas generator.hpp
//  Animera
//
//  Created by Indiana Kernick on 27/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_json_atlas_generator_hpp
#define animera_json_atlas_generator_hpp

#include <QtCore/qjsondocument.h>
#include "basic atlas generator.hpp"

class JsonAtlasGenerator final : public BasicAtlasGenerator {
public:
  JsonAtlasGenerator();

  Error initAtlas(PixelFormat, const QString &, const QString &) override;
  Error packRectangles() override;
  Error finalize() override;
  
  void appendName(const QString &, std::size_t) override;
  void appendRect(QRect) override;
  void fixName(QString &, std::array<int, 4> &) override;
  void appendAlias(QString, const char *, std::size_t) override;

private:
  QString atlas;
  QString atlasName;
  QString atlasDir;
};

#endif
