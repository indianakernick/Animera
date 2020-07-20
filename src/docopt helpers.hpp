//
//  docopt helpers.hpp
//  Animera
//
//  Created by Indiana Kernick on 30/11/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_docopt_helpers_hpp
#define animera_docopt_helpers_hpp

#include "error.hpp"
#include "int range.hpp"
#include <QtCore/qstring.h>

namespace docopt {

struct value;

}

Error setInt(long &, const docopt::value &, const QString &);
Error setInt(int &, const docopt::value &, const QString &, IntRange);

template <typename Func>
QString validListStr(const QString &name, const std::size_t length, Func func) {
  QString str = "\nValid " + name + " are: {";
  if (length != 0) {
    func(str, std::size_t{0});
    for (std::size_t i = 1; i != length; ++i) {
      str += ", ";
      func(str, i);
    }
    str += '}';
  }
  return str;
}

template <std::size_t Size>
QString validListStr(const QString &name, const char *(&names)[Size]) {
  return validListStr(name, Size, [&names](QString &str, std::size_t i) {
    str += names[i];
  });
}

template <typename Enum, std::size_t Size>
bool setEnum(Enum &value, std::string name, const char *(&names)[Size]) {
  for (char &ch : name) {
    ch = std::tolower(static_cast<unsigned char>(ch));
  }
  for (std::size_t i = 0; i != Size; ++i) {
    if (name == names[i]) {
      value = static_cast<Enum>(i);
      return true;
    }
  }
  return false;
}

#endif
