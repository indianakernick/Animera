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
#include "export backend.hpp"

// TODO: support indexed animation format

class SpritePacker {
public:
  static constexpr int padding = 1;

  void init(PixelFormat);
  void append(std::size_t, QSize);
  void appendWhite();
  
  Error pack();
  QRect copy(std::size_t, const QImage &);
  QRect copyWhite(std::size_t);
  Error writePng(QIODevice &);
  
  QRect rect(const std::size_t i) const {
    assert(i < rects.size());
    return {
      rects[i].x + padding, rects[i].y + padding,
      rects[i].w - 2 * padding, rects[i].h - 2 * padding
    };
  }
  
  std::size_t count() const {
    return rects.size();
  }
  int width() const {
    return texture.width();
  }
  int height() const {
    return texture.height();
  }
  
private:
  QImage texture;
  std::vector<stbrp_rect> rects;
  int area = 0;
  PixelFormat format;
};

#endif
