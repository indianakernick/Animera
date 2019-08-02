//
//  widget rect.hpp
//  Animera
//
//  Created by Indi Kernick on 24/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef widget_rect_hpp
#define widget_rect_hpp

#include <QtCore/qrect.h>

class WidgetRect final {
public:
  constexpr WidgetRect(QRect widget, QRect outer, QRect inner, QPoint pos)
    : widget_{widget}, outer_{outer}, inner_{inner}, pos_{pos} {}
  
  constexpr QRect widget() const {
    return widget_;
  }
  constexpr QRect outer() const {
    return outer_;
  }
  constexpr QRect inner() const {
    return inner_;
  }
  constexpr QPoint pos() const {
    return pos_;
  }

private:
  QRect widget_;
  QRect outer_;
  QRect inner_;
  QPoint pos_;
};

#endif
