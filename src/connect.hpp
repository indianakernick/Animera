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
constexpr auto address(T &&obj, int) -> decltype(&*obj) {
  return &*obj;
}

template <typename T>
constexpr auto address(T &&obj, long) {
  return &obj;
}

// It's a real shame that I have to write std::remove_reference_t
// This code looks so elegant without it
template <typename T>
constexpr auto value(T &&obj, int) -> std::remove_reference_t<decltype(*obj)>;

template <typename T>
constexpr auto value(T &&obj, long) -> std::remove_reference_t<decltype(obj)>;

}

#define CONNECT(SENDER, SIGNAL, RECEIVER, SLOT)                                 \
  QObject::connect(                                                             \
    detail::address(SENDER, 0),                                                 \
    &decltype(detail::value(SENDER, 0))::SIGNAL,                                \
    detail::address(RECEIVER, 0),                                               \
    &decltype(detail::value(RECEIVER, 0))::SLOT                                 \
  )

#define DISCONNECT(SENDER, SIGNAL, RECEIVER, SLOT)                              \
  QObject::disconnect(                                                          \
    detail::address(SENDER, 0),                                                 \
    &decltype(detail::value(SENDER, 0))::SIGNAL,                                \
    detail::address(RECEIVER, 0),                                               \
    &decltype(detail::value(RECEIVER, 0))::SLOT                                 \
  )

// connect\((.+),\s+&\w+::(\w+),\s+(.+),\s+&\w+::(\w+)\);
// CONNECT($1, $2, $3, $4);

#endif
