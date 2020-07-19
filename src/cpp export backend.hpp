//
//  cpp export backend.hpp
//  Animera
//
//  Created by Indiana Kernick on 18/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_cpp_export_backend_hpp
#define animera_cpp_export_backend_hpp

#include <unordered_set>
#include "stb_rect_pack.h"
#include "export backend.hpp"

class CppExportBackend final : public ExportBackend {
public:
  Error initAtlas(PixelFormat, const QString &, const QString &) override;
  
  void addName(std::size_t, const ExportNameParams &, const ExportNameState &) override;
  void addSizes(std::size_t, QSize) override;
  void addWhiteName() override;
  QString hasNameCollision() override;
  
  Error initAnimation(Format, PaletteCSpan) override;
  Error addImage(std::size_t, QImage) override;
  Error addWhiteImage() override;
  
  Error finalize() override;

private:
  QString enumeration;
  QString array;
  std::unordered_set<QString> names;
  QString collision;
  std::vector<stbrp_rect> rects;
  int totalArea;
  QImage texture;
  QString atlasDir;
  QString atlasName;
  
  void appendEnumerator(const QString &, const QString & = {});
  void appendRectangle(const stbrp_rect &);
  void insertName(const QString &);
  
  Error writeBytes(QIODevice &, const char *, std::size_t);
  Error writeCpp();
  Error writeHpp();
};

#endif
