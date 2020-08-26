//
//  export sprite sheet.hpp
//  Animera
//
//  Created by Indiana Kernick on 26/8/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_export_sprite_sheet_hpp
#define animera_export_sprite_sheet_hpp

#include "export params.hpp"

class NameAppender {
public:
  template <typename>
  friend auto selectFunc(const SpriteNameParams &);

  NameAppender(AtlasGenerator *, const SpriteNameParams &, QSize);

  void append(std::size_t &, const SpriteNameState &, bool) const;

private:
  decltype(&NameAppender::append) appendFunc;
  AtlasGenerator *generator;
  const SpriteNameParams &nameParams;
  QSize size;
  
  template <auto RangeFn, auto DimFn>
  void funcImpl(std::size_t &, const SpriteNameState &, bool) const;
  
  void noSheetImpl(std::size_t &, const SpriteNameState &, bool) const;
};

class ImageCopier {
public:
  template <typename>
  friend auto selectFunc(const SpriteNameParams &);

  ImageCopier(AtlasGenerator *, const SpriteNameParams &, QSize, Format);

  Error copy(std::size_t &, const SpriteNameState &, const QImage *);

private:
  decltype(&ImageCopier::copy) copyFunc;
  AtlasGenerator *generator;
  QImage sheetImage;
  QSize size;
  Format format;
  
  template <auto RangeFn, auto DimFn>
  Error funcImpl(std::size_t &, const SpriteNameState &, const QImage *);
  
  Error noSheetImpl(std::size_t &, const SpriteNameState &, const QImage *);
};

#endif
