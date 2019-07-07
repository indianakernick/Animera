//
//  connect.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef connect_hpp
#define connect_hpp

#include <type_traits>
#include <QtCore/qobject.h>

namespace detail {

template <typename T>
using remove_cvptr_t = std::remove_cv_t<std::remove_pointer_t<T>>;

template <typename T>
auto address(T &&obj) {
  if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) {
    return obj;
  } else {
    return &obj;
  }
}

}

#define CONNECT(SENDER, SIGNAL, RECEIVER, SLOT)                                 \
  QObject::connect(                                                             \
    detail::address(SENDER),                                                    \
    &detail::remove_cvptr_t<decltype(SENDER)>::SIGNAL,                          \
    detail::address(RECEIVER),                                                  \
    &detail::remove_cvptr_t<decltype(RECEIVER)>::SLOT                           \
  )

#define DISCONNECT(SENDER, SIGNAL, RECEIVER, SLOT)                              \
  QObject::disconnect(                                                          \
    detail::address(SENDER),                                                    \
    &detail::remove_cvptr_t<decltype(SENDER)>::SIGNAL,                          \
    detail::address(RECEIVER),                                                  \
    &detail::remove_cvptr_t<decltype(RECEIVER)>::SLOT                           \
  )

// connect\((.+),\s+&\w+::(\w+),\s+(.+),\s+&\w+::(\w+)\);
// CONNECT($1, $2, $3, $4);

#endif
