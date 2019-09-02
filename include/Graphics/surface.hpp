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
  
  bool containsX(const int posX) const noexcept {
    return 0 <= posX && posX < width_;
  }
  bool containsY(const int posY) const noexcept {
    return 0 <= posY && posY < height_;
  }
  bool contains(const Point pos) const noexcept {
    return containsX(pos.x) && containsY(pos.y);
  }
  bool contains(const Rect rect) const noexcept {
    return contains(rect.p) && contains(rect.p + rect.s.point() - Point{1, 1});
  }
  
  ptrdiff_t idx(const Point pos) const noexcept {
    return pos.y * pitch_ + pos.x;
  }
  Pixel *ptr(const Point pos) const noexcept {
    return data_ + idx(pos);
  }
  Pixel &ref(const Pixel pos) const noexcept {
    assert(contains(pos));
    return *ptr(pos);
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
