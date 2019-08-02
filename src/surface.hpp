//
//  surface.hpp
//  Animera
//
//  Created by Indi Kernick on 6/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef surface_hpp
#define surface_hpp

#include <cassert>
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
  std::remove_cvref_t<Begin>,
  std::remove_cvref_t<End>
>;

/// A non-owning view onto pixel data. Think of it as a 2D std::span
template <typename Pixel>
class Surface {
public:
  template <typename OtherPixel>
  friend class Surface;

  using pixel_type = Pixel;

  Surface(Pixel *data, const ptrdiff_t pitch, const int width, const int height) noexcept
    : data{data}, pitch{pitch}, width{width}, height{height} {}
  
  template <typename OtherPixel, typename = std::enable_if_t<
    std::is_convertible_v<OtherPixel (*)[], Pixel (*)[]>
  >>
  Surface(const Surface<OtherPixel> &other) noexcept
    : Surface{other.data, other.pitch, other.width, other.height} {}
  Surface(const Surface &) noexcept = default;
  
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
  Pixel *pixelAddr(const QPoint pos) const noexcept {
    return data + pixelIndex(pos);
  }
  
  // @TODO Better error messages with CRTP
  #define MODIFYING(RET)                                                        \
    template <typename Dummy = Pixel>                                           \
    std::enable_if_t<                                                           \
      !std::is_const_v<Pixel> && std::is_same_v<Dummy, Pixel>,                  \
      RET                                                                       \
    >
  
  MODIFYING(bool) setPixelClip(const Pixel color, const QPoint pos) const noexcept {
    if (insideImage(pos)) {
      *pixelAddr(pos) = color;
      return true;
    } else {
      return false;
    }
  }
  MODIFYING(void) setPixel(const Pixel color, const QPoint pos) const noexcept {
    assert(insideImage(pos));
    *pixelAddr(pos) = color;
  }
  Pixel getPixel(const QPoint pos) const noexcept {
    assert(insideImage(pos));
    return *pixelAddr(pos);
  }
  
  MODIFYING(void) fillRow(const Pixel color, Pixel *firstPixel, const ptrdiff_t count) const noexcept {
    if constexpr (sizeof(Pixel) == 1) {
      std::memset(firstPixel, color, count);
    } else {
      Pixel *const afterLastPixel = firstPixel + count;
      while (firstPixel != afterLastPixel) {
        *firstPixel++ = color;
      }
    }
  }
  MODIFYING(void) fillCol(const Pixel color, Pixel *firstPixel, const ptrdiff_t count) const noexcept {
    Pixel *const afterLastPixel = firstPixel + count * pitch;
    while (firstPixel != afterLastPixel) {
      *firstPixel = color;
      firstPixel += pitch;
    }
  }
  
  MODIFYING(void) horiLine(const Pixel color, const QPoint first, const int last) const noexcept {
    assert(first.x() <= last);
    assert(insideImage(first));
    assert(insideImage({last, first.y()}));
    fillRow(color, pixelAddr(first), last - first.x() + 1);
  }
  MODIFYING(void) vertLine(const Pixel color, const QPoint first, const int last) const noexcept {
    assert(first.y() <= last);
    assert(insideImage(first));
    assert(insideImage({first.x(), last}));
    fillCol(color, pixelAddr(first), last - first.y() + 1);
  }
  
  MODIFYING(bool) horiLineClip(const Pixel color, QPoint first, int last) const noexcept {
    if (!insideImageY(first.y())) return false;
    first.setX(std::max(first.x(), 0));
    last = std::min(last, width - 1);
    if (first.x() > last) return false;
    horiLine(color, first, last);
    return true;
  }
  MODIFYING(bool) vertLineClip(const Pixel color, QPoint first, int last) const noexcept {
    if (!insideImageX(first.x())) return false;
    first.setY(std::max(first.y(), 0));
    last = std::min(last, height - 1);
    if (first.y() > last) return false;
    vertLine(color, first, last);
    return true;
  }
  
  MODIFYING(void) fillRect(const Pixel color, const QRect rect) const noexcept {
    assert(!rect.isEmpty());
    assert(insideImage(rect.topLeft()));
    assert(insideImage(rect.bottomRight()));
    for (auto row : range(rect)) {
      fillRow(color, row.begin(), rect.width());
    }
  }
  
  MODIFYING(bool) fillRectClip(const Pixel color, QRect rect) const noexcept {
    rect.setLeft(std::max(rect.left(), 0));
    rect.setTop(std::max(rect.top(), 0));
    rect.setRight(std::min(rect.right(), width - 1));
    rect.setBottom(std::min(rect.bottom(), height - 1));
    if (rect.isEmpty()) return false;
    fillRect(color, rect);
    return true;
  }
  
  MODIFYING(void) fill(const Pixel color) const noexcept {
    for (auto row : range()) {
      fillRow(color, (*row).begin(), width);
    }
  }
  MODIFYING(void) fill() const noexcept {
    fill(0);
  }
  MODIFYING(void) overFill(const Pixel color) const noexcept {
    if constexpr (sizeof(Pixel) == 1) {
      std::memset(data, color, pitch * height);
    } else {
      for (Pixel &pixel : Row{data, pixelAddr({width, height - 1})}) {
        pixel = color;
      }
    }
  }
  MODIFYING(void) overFill() const noexcept {
    std::memset(data, 0, pitch * height * sizeof(Pixel));
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
    // @TODO make this a full random-access iterator
    // (except for the dereference operator)
    Row operator*() const noexcept {
      return {row, row + width};
    }
    Iterator &operator++() noexcept {
      row += pitch;
      return *this;
    }
    Iterator &operator+=(const ptrdiff_t diff) {
      row += pitch * diff;
      return *this;
    }
    bool operator!=(const Sentinel sentinel) const noexcept {
      return row != sentinel.row;
    }
    
    // @TODO wrapping iterator would probably be better
    Pixel *nextWrappedPixel(Pixel *pixel) const noexcept {
      return row + (pixel + 1 - row) % width;
    }
    
  private:
    Pixel *row;
    ptrdiff_t pitch;
    ptrdiff_t width;
    
    Iterator(Pixel *row, ptrdiff_t pitch, ptrdiff_t width)
      : row{row}, pitch{pitch}, width{width} {}
  };
  
  using Range = Range<Iterator, Sentinel>;
  
  Range range() const noexcept {
    return {Iterator{data, pitch, width}, Sentinel{data + pitch * height}};
  }
  Range range(const QRect rect) const noexcept {
    return {
      Iterator{pixelAddr(rect.topLeft()), pitch, rect.width()},
      Sentinel{pixelAddr({rect.left(), rect.bottom() + 1})}
    };
  }
  Row row() const noexcept {
    return {data, data + width};
  }
  Row row(const int y) const noexcept {
    return {pixelAddr({0, y}), pixelAddr({width, y})};
  }
  
  // @TODO wrapping iterator would probably be better
  Iterator nextWrappedRow(const Range &range, const Iterator &iter) {
    Pixel *const begin = range.begin().row;
    Pixel *const end = range.end().row;
    return {begin + (iter.row + iter.pitch - begin) % (iter.pitch * (end - begin)), iter.pitch, iter.width};
  }
  
  #undef MODIFYING

private:
  Pixel *data;
  ptrdiff_t pitch;
  int width;
  int height;
};

/// A non-owning view onto immutable pixel data. Think of it as a 2D std::cspan
template <typename Pixel>
using CSurface = Surface<const Pixel>;

// @TODO std::type_identity_t

template <typename T>
struct identity {
  using type = T;
};

template <typename T>
using identity_t = typename identity<T>::type;

#endif
