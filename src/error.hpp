//
//  error.hpp
//  Animera
//
//  Created by Indi Kernick on 17/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef error_hpp
#define error_hpp

#include <cassert>
#include <QtCore/qstring.h>

inline void assertEval([[maybe_unused]] const bool cond) noexcept {
  assert(cond);
}

class [[nodiscard]] Error {
public:
  Error() = default;
  Error(const QString &m)
    : m{m} {}
  
  template <size_t Size>
  Error(const char (&str)[Size])
    : m{QLatin1String{str, Size - 1}} {}
  
  Error(const char *str)
    : m{QLatin1String{str}} {}
  
  explicit operator bool() const noexcept {
    return !m.isEmpty();
  }
  
  const QString &msg() const noexcept {
    return m;
  }

private:
  QString m;
};

#define TRY(...) if (Error _error = (__VA_ARGS__)) return _error

#endif
