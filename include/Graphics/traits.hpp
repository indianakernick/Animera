//
//  traits.hpp
//  Animera
//
//  Created by Indi Kernick on 1/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_traits_hpp
#define graphics_traits_hpp

namespace gfx {

// TODO: std::type_identity_t

template <typename T>
struct identity {
  using type = T;
};

template <typename T>
using identity_t = typename identity<T>::type;

template <typename T>
using Pixel = typename T::Pixel;

}

#endif
