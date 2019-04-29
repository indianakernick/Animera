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

#define CONNECT(SENDER, SIGNAL, RECEIVER, SLOT)                                 \
  QObject::connect(                                                             \
    SENDER,                                                                     \
    &std::remove_cv_t<std::remove_pointer_t<decltype(SENDER)>>::SIGNAL,         \
    RECEIVER,                                                                   \
    &std::remove_cv_t<std::remove_pointer_t<decltype(RECEIVER)>>::SLOT          \
  )

// connect\((.+),\s+&\w+::(\w+),\s+(.+),\s+&\w+::(\w+)\);
// CONNECT($1, $2, $3, $4);

#endif
