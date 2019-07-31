//
//  enum operators.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef enum_operators_hpp
#define enum_operators_hpp

#include <type_traits>

template <typename Enum>
struct enum_unary_plus : std::false_type {};

template <typename Enum>
struct enum_bitwise : std::false_type {};

template <typename Enum>
struct enum_bitwise_not : enum_bitwise<Enum> {};

template <typename Enum>
struct enum_bitwise_and : enum_bitwise<Enum> {};

template <typename Enum>
struct enum_bitwise_or : enum_bitwise<Enum> {};

template <typename Enum>
struct enum_bitwise_xor : enum_bitwise<Enum> {};

template <typename Enum>
struct enum_bool : std::false_type {};

template <typename Enum>
struct enum_bool_not : enum_bool<Enum> {};

template <typename Enum>
struct enum_bool_and : enum_bool<Enum> {};

template <typename Enum>
struct enum_bool_or : enum_bool<Enum> {};

template <typename Enum>
struct enum_flag : std::false_type {};

template <typename Enum>
struct enum_set_flag : enum_flag<Enum> {};

template <typename Enum>
struct enum_reset_flag : enum_flag<Enum> {};

template <typename Enum>
struct enum_flip_flag : enum_flag<Enum> {};

template <typename Enum>
struct enum_test_flag : enum_flag<Enum> {};

template <typename Enum>
struct enum_math : std::false_type {};

template <typename Enum>
struct enum_math_neg : enum_math<Enum> {};

template <typename Enum>
struct enum_math_add_sub : enum_math<Enum> {};

template <typename Enum>
struct enum_math_mul_div_mod : enum_math<Enum> {};

template <typename Enum>
struct enum_math_inc_dec : enum_math<Enum> {};

namespace detail {

template <typename Enum>
constexpr auto to_int(const Enum e) noexcept {
  return static_cast<std::underlying_type_t<Enum>>(e);
}

}

template <typename Enum>
constexpr std::enable_if_t<
  enum_unary_plus<Enum>::value,
  std::underlying_type_t<Enum>
>
operator+(const Enum e) noexcept {
  return detail::to_int(e);
}

#define BINARY_OP(ENABLE, OP)                                                   \
  template <typename Enum>                                                      \
  constexpr std::enable_if_t<ENABLE<Enum>::value, Enum>                         \
  operator OP(const Enum a, const Enum b) noexcept {                            \
    return static_cast<Enum>(detail::to_int(a) OP detail::to_int(b));           \
  }                                                                             \
                                                                                \
  template <typename Enum>                                                      \
  constexpr std::enable_if_t<ENABLE<Enum>::value, Enum &>                       \
  operator OP##=(Enum &a, const Enum b) noexcept {                              \
    return a = a OP b;                                                          \
  }

template <typename Enum>
constexpr std::enable_if_t<enum_bitwise_not<Enum>::value, Enum>
operator~(const Enum e) noexcept {
  return static_cast<Enum>(~detail::to_int(e));
}

BINARY_OP(enum_bitwise_and, &)
BINARY_OP(enum_bitwise_or, |)
BINARY_OP(enum_bitwise_xor, ^)

template <typename Enum>
constexpr std::enable_if_t<enum_bool_not<Enum>::value, bool>
operator!(const Enum e) noexcept {
  return !static_cast<bool>(e);
}

template <typename Enum>
constexpr std::enable_if_t<enum_bool_and<Enum>::value, bool>
operator&&(const Enum a, const Enum b) noexcept {
  return static_cast<bool>(a) && static_cast<bool>(b);
}

template <typename Enum>
constexpr std::enable_if_t<enum_bool_or<Enum>::value, bool>
operator||(const Enum a, const Enum b) noexcept {
  return static_cast<bool>(a) || static_cast<bool>(b);
}

template <typename Enum>
constexpr std::enable_if_t<enum_set_flag<Enum>::value, Enum>
set_flag(const Enum a, const Enum b) noexcept {
  return static_cast<Enum>(detail::to_int(a) | detail::to_int(b));
}

template <typename Enum>
constexpr std::enable_if_t<enum_reset_flag<Enum>::value, Enum>
reset_flag(const Enum a, const Enum b) noexcept {
  return static_cast<Enum>(detail::to_int(a) & ~detail::to_int(b));
}

template <typename Enum>
constexpr std::enable_if_t<enum_flip_flag<Enum>::value, Enum>
flip_flag(const Enum a, const Enum b) noexcept {
  return static_cast<Enum>(detail::to_int(a) ^ detail::to_int(b));
}

template <typename Enum>
constexpr std::enable_if_t<enum_test_flag<Enum>::value, bool>
test_flag(const Enum a, const Enum b) noexcept {
  return static_cast<bool>(detail::to_int(a) & detail::to_int(b));
}

template <typename Enum>
constexpr std::enable_if_t<enum_math_neg<Enum>::value, Enum>
operator-(const Enum e) noexcept {
  return static_cast<Enum>(-detail::to_int(e));
}

BINARY_OP(enum_math_add_sub, +)
BINARY_OP(enum_math_add_sub, -)
BINARY_OP(enum_math_mul_div_mod, *)
BINARY_OP(enum_math_mul_div_mod, /)
BINARY_OP(enum_math_mul_div_mod, %)

template <typename Enum>
constexpr std::enable_if_t<enum_math_inc_dec<Enum>::value, Enum &>
operator++(Enum &e) noexcept {
  return e = static_cast<Enum>(detail::to_int(e) + 1);
}

template <typename Enum>
constexpr std::enable_if_t<enum_math_inc_dec<Enum>::value, Enum>
operator++(Enum &e, int) noexcept {
  const Enum old = e;
  ++e;
  return old;
}

template <typename Enum>
constexpr std::enable_if_t<enum_math_inc_dec<Enum>::value, Enum &>
operator--(Enum &e) noexcept {
  return e = static_cast<Enum>(detail::to_int(e) - 1);
}

template <typename Enum>
constexpr std::enable_if_t<enum_math_inc_dec<Enum>::value, Enum>
operator--(Enum &e, int) noexcept {
  const Enum old = e;
  --e;
  return old;
}

#undef BINARY_OP

#endif
