//
//  color handle.hpp
//  Animera
//
//  Created by Indiana Kernick on 30/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_color_handle_hpp
#define animera_color_handle_hpp

#include "image.hpp"
#include <QtCore/qstring.h>

class ColorHandle {
protected:
  ~ColorHandle() = default;

public:
  virtual PixelVar getInitialColor() const = 0;
  virtual void setColor(PixelVar) = 0;
  virtual QString getName() const = 0;
  virtual void detach() = 0;
};

#endif
