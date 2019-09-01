//
//  range.hpp
//  Animera
//
//  Created by Indi Kernick on 1/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_range_hpp
#define graphics_range_hpp

#include <type_traits>

namespace gfx {

template <typename Begin, typename End = Begin>
class Range {
public:
  Range(const Begin &begin, const End &end)
    : begin_{begin}, end_{end} {}
  
  Begin begin() const noexcept {
    return begin_;
  }
  End end() const noexcept {
    return end_;
  }

private:
  Begin begin_;
  End end_;
};

template <typename Begin, typename End>
Range(Begin &&, End &&) -> Range<
  std::remove_cvref_t<Begin>,
  std::remove_cvref_t<End>
>;

}

#endif
