//
//  surface.hpp
//  Animera
//
//  Created by Indi Kernick on 1/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_surface_hpp
#define graphics_surface_hpp

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
    : data_{data}, pitch_{pitch}, width_{width}, height_{height} {}
  
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
  
  template <typename OtherPixel>
  OtherPixel *dataAs() const noexcept {
    return reinterpret_cast<OtherPixel *>(data_);
  }
  template <typename OtherPixel>
  ptrdiff_t pitchAs() const noexcept {
    return pitch_ * ptrdiff_t{sizeof(Pixel)} / ptrdiff_t{sizeof(OtherPixel)};
  }
  
  bool insideX(const int posX) const noexcept {
    return 0 <= posX && posX < width_;
  }
  bool insideY(const int posY) const noexcept {
    return 0 <= posY && posY < height_;
  }
  bool inside(const Point pos) const noexcept {
    return insideX(pos.x) && insideY(pos.y);
  }
  
  ptrdiff_t pixelIndex(const Point pos) const noexcept {
    return pos.y * pitch_ + pos.x;
  }
  Pixel *pixelAddr(const Point pos) const noexcept {
    return data_ + pixelIndex(pos);
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
