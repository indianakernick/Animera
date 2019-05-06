//
//  pixel manip.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 6/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef pixel_manip_hpp
#define pixel_manip_hpp

#include <cstring>
#include <algorithm>
#include <QtCore/qrect.h>

// @TODO maybe we could call this a Surface?
template <typename Pixel>
class PixelManip {
public:
  PixelManip(Pixel *data, const ptrdiff_t pitch, const int width, const int height)
    : data{data}, pitch{pitch}, width{width}, height{height} {}
  
  bool insideImageX(const int posX) const {
    return 0 <= posX && posX < width;
  }
  bool insideImageY(const int posY) const {
    return 0 <= posY && posY < height;
  }
  bool insideImage(const QPoint pos) const {
    return insideImageX(pos.x()) && insideImageY(pos.y());
  }
  int clipX(const int posX) const {
    return std::clamp(posX, 0, width);
  }
  int clipY(const int posY) const {
    return std::clamp(posY, 0, height);
  }
  
  ptrdiff_t pixelIndex(const QPoint pos) const {
    return pos.y() * pitch + pos.x();
  }
  Pixel *pixelAddr(const QPoint pos) {
    return data + pixelIndex(pos);
  }
  const Pixel *pixelAddr(const QPoint pos) const {
    return data + pixelIndex(pos);
  }
  
  bool setPixelClip(const Pixel color, const QPoint pos) {
    if (insideImage(pos)) {
      *pixelAddr(pos) = color;
      return true;
    } else {
      return false;
    }
  }
  void setPixel(const Pixel color, const QPoint pos) {
    assert(insideImage(pos));
    *pixelAddr(pos) = color;
  }
  Pixel getPixel(const QPoint pos) const {
    assert(insideImage(pos));
    return *pixelAddr(pos);
  }
  
  void fillRow(const Pixel color, Pixel *firstPixel, const ptrdiff_t count) {
    if constexpr (std::is_same_v<Pixel, uint8_t>) {
      std::memset(firstPixel, color, count);
    } else {
      Pixel *const afterLastPixel = firstPixel + count;
      while (firstPixel != afterLastPixel) {
        *firstPixel++ = color;
      }
    }
  }
  void fillCol(const Pixel color, Pixel *firstPixel, const ptrdiff_t count) {
    Pixel *const afterLastPixel = firstPixel + count * pitch;
    while (firstPixel != afterLastPixel) {
      *firstPixel = color;
      firstPixel += pitch;
    }
  }
  
  void horiLine(const Pixel color, const QPoint first, const int last) {
    assert(first.x() <= last);
    assert(insideImage(first));
    assert(insideImage({first.x() + last, first.y()}));
    fillRow(color, pixelAddr(first), last - first.x() + 1);
  }
  void vertLine(const Pixel color, const QPoint first, const int last) {
    assert(first.y() <= last);
    assert(insideImage(first));
    assert(insideImage({first.x(), first.y() + last}));
    fillCol(color, pixelAddr(first), last - first.y() + 1);
  }
  
  bool horiLineClip(const Pixel color, QPoint first, int last) {
    if (!insideImageY(first.y())) return false;
    first.setX(clipX(first.x()));
    last = clipX(last);
    if (first.x() > last) return false;
    horiLine(color, first, last);
    return true;
  }
  bool vertLineClip(const Pixel color, QPoint first, int last) {
    if (!insideImageX(first.x())) return false;
    first.setY(clipY(first.y()));
    last = clipY(last);
    if (first.y() > last) return false;
    vertLine(color, first, last);
    return true;
  }
  
  void fillRect(const Pixel color, const QRect rect) {
    assert(!rect.isEmpty());
    assert(insideImage(rect.topLeft()));
    assert(insideImage(rect.bottomRight()));
    Pixel *firstRow = pixelAddr(rect.topLeft());
    Pixel *const afterLastRow = pixelAddr({rect.left(), rect.bottom() + 1});
    const ptrdiff_t rowWidth = rect.width();
    while (firstRow != afterLastRow) {
      fillRow(color, firstRow, rowWidth);
      firstRow += pitch;
    }
  }
  
  bool fillRectClip(const Pixel color, QRect rect) {
    rect.setLeft(clipX(rect.left()));
    rect.setTop(clipY(rect.top()));
    rect.setRight(clipX(rect.right()));
    rect.setBottom(clipY(rect.bottom()));
    if (rect.isEmpty()) return false;
    fillRect(color, rect);
    return true;
  }

private:
  Pixel *data;
  ptrdiff_t pitch;
  int width;
  int height;
};

#endif
