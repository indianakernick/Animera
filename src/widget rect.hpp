//
//  widget rect.hpp
//  Pixel 2
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

// @TODO TextIconRects isn't the right solution
class TextIconRects final {
public:
  constexpr TextIconRects(
    QRect widget,
    QRect outer,
    QRect textInner,
    QRect iconInner,
    QPoint textPos,
    QPoint iconPos,
    QRect border
  ) : widget_{widget},
      outer_{outer},
      textInner_{textInner},
      iconInner_{iconInner},
      textPos_{textPos},
      iconPos_{iconPos},
      border_{border} {}
  
  constexpr WidgetRect text() const {
    return {widget_, outer_, textInner_, textPos_};
  }
  constexpr WidgetRect icon() const {
    return {widget_, outer_, iconInner_, iconPos_};
  }
  
  constexpr QRect widget() const {
    return widget_;
  }
  constexpr QRect outer() const {
    return outer_;
  }
  constexpr QRect textInner() const {
    return textInner_;
  }
  constexpr QRect iconInner() const {
    return iconInner_;
  }
  constexpr QPoint textPos() const {
    return textPos_;
  }
  constexpr QPoint iconPos() const {
    return iconPos_;
  }
  constexpr QRect border() const {
    return border_;
  }

private:
  QRect widget_;
  QRect outer_;
  QRect textInner_;
  QRect iconInner_;
  QPoint textPos_;
  QPoint iconPos_;
  QRect border_;
};

#endif
