//
//  iterator.hpp
//  Animera
//
//  Created by Indi Kernick on 1/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_iterator_hpp
#define graphics_iterator_hpp

#include <cassert>
#include <iterator>
#include "surface.hpp"

namespace gfx {

template <typename Pixel>
using ColIterator = Pixel *;

template <typename Pixel>
using ColRange = Range<ColIterator<Pixel>>;

template <typename Pixel>
class RowIterator {
public:
  using value_type = ColRange<Pixel>;
  using difference_type = ptrdiff_t;
  using reference = value_type &;
  using pointer = value_type *;
  using iterator_category = std::random_access_iterator_tag;

  RowIterator() noexcept
    : data{nullptr}, pitch{0}, width{0} {}
  RowIterator(Pixel *data, const ptrdiff_t pitch, const ptrdiff_t width) noexcept
    : data{data}, pitch{pitch}, width{width} {}

  // This is not technically a random access iterator
  value_type operator*() const noexcept {
    return {data, data + width};
  }
  value_type operator[](const difference_type idx) const noexcept {
    ColIterator<Pixel> begin = data + pitch * idx;
    return {begin, begin + width};
  }
  
  ColIterator<Pixel> begin() const noexcept {
    return data;
  }
  ColIterator<const Pixel> cbegin() const noexcept {
    return data;
  }
  ColIterator<Pixel> end() const noexcept {
    return data + width;
  }
  ColIterator<const Pixel> cend() const noexcept {
    return data + width;
  }

  bool operator==(const RowIterator &other) const noexcept {
    return data == other.data && pitch == other.pitch;
  }
  bool operator!=(const RowIterator &other) const noexcept {
    return !(*this == other);
  }
  bool operator<(const RowIterator &other) const noexcept {
    return data < other.data;
  }
  bool operator>(const RowIterator &other) const noexcept {
    return data > other.data;
  }
  bool operator<=(const RowIterator &other) const noexcept {
    return data <= other.data;
  }
  bool operator>=(const RowIterator &other) const noexcept {
    return data >= other.data;
  }
  
  RowIterator &operator++() noexcept {
    data += pitch;
    return *this;
  }
  RowIterator operator++(int) noexcept {
    RowIterator copy = *this;
    operator++();
    return copy;
  }
  RowIterator &operator--() noexcept {
    data -= pitch;
    return *this;
  }
  RowIterator operator--(int) noexcept {
    RowIterator copy = *this;
    operator--();
    return copy;
  }
  
  RowIterator &operator+=(const difference_type diff) noexcept {
    data += pitch * diff;
    return *this;
  }
  RowIterator &operator-=(const difference_type diff) noexcept {
    data -= pitch * diff;
    return *this;
  }
  
  difference_type operator-(const RowIterator &other) const noexcept {
    assert(pitch == other.pitch);
    return (data - other.data) / pitch;
  }
  RowIterator operator-(const difference_type diff) const noexcept {
    return {data - pitch * diff, pitch, width};
  }
  
  RowIterator operator+(const difference_type diff) const noexcept {
    return {data + pitch * diff, pitch, width};
  }

private:
  Pixel *data;
  ptrdiff_t pitch;
  ptrdiff_t width;
};

template <typename Pixel>
RowIterator<Pixel> operator+(
  const typename RowIterator<Pixel>::difference_type diff,
  const RowIterator<Pixel> &iter
) noexcept {
  return iter + diff;
}

template <typename Pixel>
using RowRange = Range<RowIterator<Pixel>>;

template <typename Pixel>
RowRange<Pixel> range(const Surface<Pixel> &surface) noexcept {
  return {
    {surface.data(), surface.pitch(), surface.width()},
    {surface.data() + surface.pitch() * surface.height(), surface.pitch(), surface.width()}
  };
}

template <typename Pixel>
RowRange<Pixel> range(const Surface<Pixel> &surface, const Rect rect) noexcept {
  return {
    {surface.pixelAddr(rect.p), surface.pitch(), rect.s.w},
    {surface.pixelAddr({rect.p.x, rect.p.y + rect.s.h}), surface.pitch(), rect.s.w}
  };
}

}

#endif
