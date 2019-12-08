//
//  connect.hpp
//  Animera
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
constexpr auto address(T &&obj, int) -> decltype(&*obj) {
  return &*obj;
}

template <typename T>
constexpr auto address(T &&obj, long) {
  return &obj;
}

template <typename T>
constexpr auto value(T &&obj, int) -> std::decay_t<decltype(*obj)>;

template <typename T>
constexpr auto value(T &&obj, long) -> std::decay_t<decltype(obj)>;

}

#define CONNECT_TYPE(SENDER, SIGNAL, RECEIVER, SLOT, TYPE)                      \
  QObject::connect(                                                             \
    detail::address(SENDER, 0),                                                 \
    &decltype(detail::value(SENDER, 0))::SIGNAL,                                \
    detail::address(RECEIVER, 0),                                               \
    &decltype(detail::value(RECEIVER, 0))::SLOT,                                \
    TYPE                                                                        \
  )

#define CONNECT(SENDER, SIGNAL, RECEIVER, SLOT)                                 \
  CONNECT_TYPE(SENDER, SIGNAL, RECEIVER, SLOT, Qt::AutoConnection)

#define CONNECT_LAMBDA(SENDER, SIGNAL, ...)                                     \
  QObject::connect(                                                             \
    detail::address(SENDER, 0),                                                 \
    &decltype(detail::value(SENDER, 0))::SIGNAL,                                \
    __VA_ARGS__                                                                 \
  )

// TODO: this is currently unused
#define CONNECT_OVERLOAD(SENDER, SIGNAL, RECEIVER, SLOT, ...)                   \
  QObject::connect(                                                             \
    detail::address(SENDER, 0),                                                 \
    qOverload<__VA_ARGS__>(&decltype(detail::value(SENDER, 0))::SIGNAL),        \
    detail::address(RECEIVER, 0),                                               \
    qOverload<__VA_ARGS__>(&decltype(detail::value(RECEIVER, 0))::SLOT)         \
  )

// connect\((.+),\s+&\w+::(\w+),\s+(.+),\s+&\w+::(\w+)\);
// CONNECT($1, $2, $3, $4);

#endif
