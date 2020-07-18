//
//  cel painter.hpp
//  Animera
//
//  Created by Indiana Kernick on 11/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef cel_painter_hpp
#define cel_painter_hpp

#include "cel.hpp"
#include <QtGui/qpixmap.h>

class CelPainter {
public:
  CelPainter();

  void start(int, int);
  void start();
  
  void advance(FrameIdx = FrameIdx{1});
  void span(QPainter &, FrameIdx) const;
  void border(QPainter &) const;
  
  int posX() const;
  int posY() const;
  
private:
  QPixmap celPix;
  QPixmap beginLinkPix;
  QPixmap endLinkPix;
  int x, y;
};

#endif
