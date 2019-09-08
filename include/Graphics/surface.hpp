//
//  surface.hpp
//  Animera
//
//  Created by Indi Kernick on 1/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_surface_hpp
#define graphics_surface_hpp

#include <cassert>
#include <type_traits>
#include "geometry.hpp"

namespace gfx {

/// A non-owning view onto pixel data
template <typename Pixel_>
class Surface {
public:
  template <typename OtherPixel>
  friend class Surface;
  
  using Pixel = Pixel_;
  
  static_assert(std::is_integral_v<Pixel>);
  static_assert(std::is_unsigned_v<Pixel>);
  
  Surface(Pixel *data, const ptrdiff_t pitch, const int width, const int height) noexcept
    : data_{data}, pitch_{pitch}, width_{width}, height_{height} {
    assert(data != nullptr);
    assert(pitch > 0);
    assert(width > 0);
    assert(height > 0);
  }
  Surface(Pixel *data, const ptrdiff_t pitch, const Size size) noexcept
    : Surface{data, pitch, size.w, size.h} {}
  
  template <typename OtherPixel, typename = std::enable_if_t<
    std::is_convertible_v<OtherPixel (*)[], Pixel (*)[]>
  >>
  Surface(const Surface<OtherPixel> &other) noexcept
    : Surface{other.data_, other.pitch_, other.width_, other.height_} {}
  Surface(const Surface &) noexcept = default;
  
  Pixel *data() const noexcept {
    return data_;
  }
  ptrdiff_t pitch() const noexcept {
    return pitch_;
  }
  int width() const noexcept {
    return width_;
  }
  int height() const noexcept {
    return height_;
  }
  
  Size size() const noexcept {
    return {width_, height_};
  }
  Rect rect() const noexcept {
    return {{}, size()};
  }
  
  template <typename OtherPixel>
  OtherPixel *dataAs() const noexcept {
    return reinterpret_cast<OtherPixel *>(data_);
  }
  template <typename OtherPixel>
  ptrdiff_t pitchAs() const noexcept {
    return pitch_ * ptrdiff_t{sizeof(Pixel)} / ptrdiff_t{sizeof(OtherPixel)};
  }
  
  bool containsX(const int x) const noexcept {
    return 0 <= x && x < width_;
  }
  bool containsY(const int y) const noexcept {
    return 0 <= y && y < height_;
  }
  bool contains(const int x, const int y) const noexcept {
    return containsX(x) && containsY(y);
  }
  bool contains(const Point pos) const noexcept {
    return contains(pos.x, pos.y);
  }
  bool contains(const Rect r) const noexcept {
    return rect().contains(r);
  }
  
  ptrdiff_t idx(const int x, const int y) const noexcept {
    return y * pitch_ + x;
  }
  Pixel *ptr(const int x, const int y) const noexcept {
    return data_ + idx(x, y);
  }
  Pixel &ref(const int x, const int y) const noexcept {
    assert(contains(x, y));
    return *ptr(x, y);
  }
  
  ptrdiff_t idx(const Point pos) const noexcept {
    return idx(pos.x, pos.y);
  }
  Pixel *ptr(const Point pos) const noexcept {
    return ptr(pos.x, pos.y);
  }
  Pixel &ref(const Point pos) const noexcept {
    return ref(pos.x, pos.y);
  }
  
  Surface view(const Rect rect) const noexcept {
    assert(contains(rect));
    return {ptr(rect.p), pitch_, rect.s};
  }
  Surface view(const Size size) const noexcept {
    assert(contains({{}, size}));
    return {data_, pitch_, size};
  }

private:
  Pixel *data_;
  ptrdiff_t pitch_;
  int width_;
  int height_;
};

/// A non-owning view onto immutable pixel data
template <typename Pixel>
using CSurface = Surface<const Pixel>;

}

#endif
