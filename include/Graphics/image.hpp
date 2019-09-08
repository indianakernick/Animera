//
//  image.hpp
//  Animera
//
//  Created by Indi Kernick on 8/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_image_hpp
#define graphics_image_hpp

#include <cstddef>

namespace gfx {

class Image {
public:
  Image() noexcept
    : data_{}, pitch_{}, width_{}, height_{}, depth_{} {}
  Image(Image &&other) noexcept
    : data_{other.data_},
      pitch_{other.pitch_},
      width_{other.width_},
      height_{other.height_},
      depth_{other.depth_} {
    other.data_ = nullptr;
    other.pitch_ = 0;
    other.width_ = 0;
    other.height_ = 0;
    other.depth_ = 0;
  }
  Image &operator=(Image &&other) noexcept {
    operator delete(data_);
    data_ = other.data_;
    pitch_ = other.pitch_;
    width_ = other.width_;
    height_ = other.height_;
    depth_ = other.depth_;
    other.data_ = nullptr;
    other.pitch_ = 0;
    other.width_ = 0;
    other.height_ = 0;
    other.depth_ = 0;
    return *this;
  }
  ~Image() noexcept {
    operator delete(data_);
  }
  
  void *data() noexcept {
    return data_;
  }
  const void *data() const noexcept {
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
  int depth() const noexcept {
    return depth_;
  }
  
private:
  void *data_;
  ptrdiff_t pitch_;
  int width_;
  int height_;
  int depth_;
};

}

#endif
