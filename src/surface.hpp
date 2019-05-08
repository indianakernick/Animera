//
//  surface.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 6/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef surface_hpp
#define surface_hpp

#include <cstring>
#include <algorithm>
#include <type_traits>
#include <QtCore/qrect.h>

template <typename Begin, typename End = Begin>
class Range {
public:
  Range(const Begin &begin, const End &end)
    : beginIter{begin},
      endIter{end} {}
  
  Begin begin() const noexcept {
    return beginIter;
  }
  End end() const noexcept {
    return endIter;
  }

private:
  Begin beginIter;
  End endIter;
};

template <typename Begin, typename End>
Range(Begin &&, End &&) -> Range<
  std::remove_cv_t<std::remove_reference_t<Begin>>,
  std::remove_cv_t<std::remove_reference_t<End>>
>;

template <typename Pixel>
class Surface {
public:
  using pixel_type = Pixel;

  Surface(Pixel *data, const ptrdiff_t pitch, const int width, const int height) noexcept
    : data{data}, pitch{pitch}, width{width}, height{height} {}
  
  template <typename Dummy = Pixel>
  std::enable_if_t<
    !std::is_const_v<Pixel> && std::is_same_v<Dummy, Pixel>,
    Surface<const Pixel>
  >
  addConst() const noexcept {
    return {data, pitch, width, height};
  }
  
  template <typename Dummy = Pixel>
  operator std::enable_if_t<
    !std::is_const_v<Pixel> && std::is_same_v<Dummy, Pixel>,
    Surface<const Pixel>
  >
  () const noexcept {
    return {data, pitch, width, height};
  }
  
  QSize size() const noexcept {
    return {width, height};
  }
  QRect rect() const noexcept {
    return {{}, size()};
  }
  
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
    Pixel *const afterLastPixel = firstPixel + count;
    while (firstPixel != afterLastPixel) {
      *firstPixel++ = color;
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
    assert(insideImage({last, first.y()}));
    fillRow(color, pixelAddr(first), last - first.x() + 1);
  }
  void vertLine(const Pixel color, const QPoint first, const int last) noexcept {
    assert(first.y() <= last);
    assert(insideImage(first));
    assert(insideImage({first.x(), last}));
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
    for (auto row : range(rect)) {
      for (Pixel &pixel : row) {
        pixel = color;
      }
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
  
  void fill(const Pixel color) noexcept {
    for (auto row : range()) {
      for (Pixel &pixel : row) {
        pixel = color;
      }
    }
  }
  void fill() noexcept {
    fill(0);
  }
  void overFill(const Pixel color) noexcept {
    for (Pixel &pixel : Row{data, pixelAddr({width, height - 1})}) {
      pixel = color;
    }
  }
  void overFill() noexcept {
    std::memset(data, 0, pitch * height);
  }
  
  using Row = Range<Pixel *>;
  
  class Sentinel {
    friend class Iterator;
    friend Surface;
    
    Pixel *row;
    
    explicit Sentinel(Pixel *row)
      : row{row} {}
  };
  
  class Iterator {
    friend Surface;
  public:
    Row operator*() const noexcept {
      return {row, row + width};
    }
    Iterator &operator++() noexcept {
      row += pitch;
      return *this;
    }
    bool operator!=(const Sentinel sentinel) const noexcept {
      return row != sentinel.row;
    }
    
  private:
    Pixel *row;
    ptrdiff_t pitch;
    ptrdiff_t width;
    
    Iterator(Pixel *row, ptrdiff_t pitch, ptrdiff_t width)
      : row{row}, pitch{pitch}, width{width} {}
  };
  
  using Range = Range<Iterator, Sentinel>;
  
  Range range() noexcept {
    return {Iterator{data, pitch, width}, Sentinel{data + pitch * height}};
  }
  Range range(const QRect rect) noexcept {
    return {
      Iterator{pixelAddr(rect.topLeft()), pitch, rect.width()},
      Sentinel{pixelAddr({rect.left(), rect.bottom() + 1})}
    };
  }
  Row row() noexcept {
    return {data, data + width};
  }
  Row row(const int y) noexcept {
    return {pixelAddr({0, y}), pixelAddr({width, y})};
  }

private:
  Pixel *data;
  ptrdiff_t pitch;
  int width;
  int height;
};

#endif
