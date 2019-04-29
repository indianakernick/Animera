//
//  rect widget size.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef rect_widget_size_hpp
#define rect_widget_size_hpp

#include "geometry.hpp"

class RectWidgetSize {
public:
  constexpr RectWidgetSize(const int padding, const int border, const QSize content)
    : padding_{padding}, border_{border}, content_{content} {}

  constexpr QRect widget() const noexcept {
    return {
      QPoint{},
      content_ + toSize(2 * border_ + 2 * padding_)
    };
  }
  
  constexpr QRect outer() const noexcept {
    return {
      toPoint(padding_),
      content_ + toSize(2 * border_)
    };
  }
  
  constexpr QRect inner() const noexcept {
    return {
      toPoint(padding_ + border_),
      content_
    };
  }
  
  constexpr int padding() const noexcept {
    return padding_;
  }
  
  constexpr int border() const noexcept {
    return border_;
  }
  
  constexpr QSize content() const noexcept {
    return content_;
  }

private:
  int padding_;
  int border_;
  QSize content_;
};

#endif
