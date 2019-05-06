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
  PixelManip(Pixel *data, const ptrdiff_t pitch, const int width, const int height) noexcept
    : data{data}, pitch{pitch}, width{width}, height{height} {}
  
  bool insideImageX(const int posX) const noexcept {
    return 0 <= posX && posX < width;
  }
  bool insideImageY(const int posY) const noexcept {
    return 0 <= posY && posY < height;
  }
  bool insideImage(const QPoint pos) const noexcept {
    return insideImageX(pos.x()) && insideImageY(pos.y());
  }
  
  ptrdiff_t pixelIndex(const QPoint pos) const noexcept {
    return pos.y() * pitch + pos.x();
  }
  Pixel *pixelAddr(const QPoint pos) noexcept {
    return data + pixelIndex(pos);
  }
  const Pixel *pixelAddr(const QPoint pos) const noexcept {
    return data + pixelIndex(pos);
  }
  
  bool setPixelClip(const Pixel color, const QPoint pos) noexcept {
    if (insideImage(pos)) {
      *pixelAddr(pos) = color;
      return true;
    } else {
      return false;
    }
  }
  void setPixel(const Pixel color, const QPoint pos) noexcept {
    assert(insideImage(pos));
    *pixelAddr(pos) = color;
  }
  Pixel getPixel(const QPoint pos) const noexcept {
    assert(insideImage(pos));
    return *pixelAddr(pos);
  }
  
  void fillRow(const Pixel color, Pixel *firstPixel, const ptrdiff_t count) noexcept {
    if constexpr (std::is_same_v<Pixel, uint8_t>) {
      std::memset(firstPixel, color, count);
    } else {
      Pixel *const afterLastPixel = firstPixel + count;
      while (firstPixel != afterLastPixel) {
        *firstPixel++ = color;
      }
    }
  }
  void fillCol(const Pixel color, Pixel *firstPixel, const ptrdiff_t count) noexcept {
    Pixel *const afterLastPixel = firstPixel + count * pitch;
    while (firstPixel != afterLastPixel) {
      *firstPixel = color;
      firstPixel += pitch;
    }
  }
  
  void horiLine(const Pixel color, const QPoint first, const int last) noexcept {
    assert(first.x() <= last);
    assert(insideImage(first));
    assert(insideImage({first.x() + last, first.y()}));
    fillRow(color, pixelAddr(first), last - first.x() + 1);
  }
  void vertLine(const Pixel color, const QPoint first, const int last) noexcept {
    assert(first.y() <= last);
    assert(insideImage(first));
    assert(insideImage({first.x(), first.y() + last}));
    fillCol(color, pixelAddr(first), last - first.y() + 1);
  }
  
  bool horiLineClip(const Pixel color, QPoint first, int last) noexcept {
    if (!insideImageY(first.y())) return false;
    first.setX(std::max(first.x(), 0));
    last = std::min(last, width - 1);
    if (first.x() > last) return false;
    horiLine(color, first, last);
    return true;
  }
  bool vertLineClip(const Pixel color, QPoint first, int last) noexcept {
    if (!insideImageX(first.x())) return false;
    first.setY(std::max(first.y(), 0));
    last = std::min(last, height - 1);
    if (first.y() > last) return false;
    vertLine(color, first, last);
    return true;
  }
  
  void fillRect(const Pixel color, const QRect rect) noexcept {
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
  
  bool fillRectClip(const Pixel color, QRect rect) noexcept {
    rect.setLeft(std::max(rect.left(), 0));
    rect.setTop(std::max(rect.top(), 0));
    rect.setRight(std::min(rect.right(), width - 1));
    rect.setBottom(std::min(rect.bottom(), height - 1));
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
