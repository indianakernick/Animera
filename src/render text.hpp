//
//  render text.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef render_text_hpp
#define render_text_hpp

#include <string>

class QPainter;

void renderText(QPainter &, int, int, const std::string &);

#endif
