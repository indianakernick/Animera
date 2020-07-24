//
//  sprite packer.hpp
//  Animera
//
//  Created by Indiana Kernick on 19/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_sprite_packer_hpp
#define animera_sprite_packer_hpp

#include <vector>
#include "error.hpp"
#include <QtCore/qrect.h>
#include "stb_rect_pack.h"

// TODO: Move PixelFormat to a more appropriate header
#include "atlas generator.hpp"

class SpritePacker {
public:
  static constexpr int padding = 1;

  void init(PixelFormat);
  void append(QSize);
  void appendWhite();
  
  Error pack();
  Error setFormat(Format, PaletteCSpan);
  QRect copy(std::size_t, const QImage &);
  QRect copyWhite(std::size_t);
  Error writePng(QIODevice &);
  
  QRect rect(std::size_t) const;
  std::size_t count() const;
  int width() const;
  int height() const;
  
private:
  QImage texture;
  std::vector<stbrp_rect> rects;
  int area = 0;
  PixelFormat pixelFormat;
  PaletteCSpan palette;
  
  using CopyFunc = void (SpritePacker::*)(const QImage &, QPoint);
  
  CopyFunc copyFunc = nullptr;
  
  CopyFunc getCopyFunc(Format) const;
  void copyRgbaToRgba(const QImage &, QPoint);
  void copyIndexToRgba(const QImage &, QPoint);
  void copyGrayToRgba(const QImage &, QPoint);
  void copyGrayToGray(const QImage &, QPoint);
  void copyGrayToGrayAlpha(const QImage &, QPoint);
};

#endif
