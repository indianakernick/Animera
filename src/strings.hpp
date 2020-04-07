//
//  strings.hpp
//  Animera
//
//  Created by Indiana Kernick on 6/11/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef strings_hpp
#define strings_hpp

#include <QtCore/qstring.h>

constexpr QLatin1String toLatinString(const std::string_view text) {
  return QLatin1String{text.data(), static_cast<int>(text.size())};
}

template <std::size_t Size>
constexpr QLatin1String toLatinString(const char (&text)[Size]) {
  return QLatin1String{text, Size - 1};
}

#endif
