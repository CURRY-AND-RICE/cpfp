#pragma once

#include "cpfp/detail/partial_n.hpp"

#include <functional>

namespace cpfp {

/// `add(L lhs, R rhs) -> decltype(lhs + rhs)`
inline constexpr auto add = detail::partial_n<2>(std::plus<>{});
/// `sub(L lhs, R rhs) -> decltype(lhs - rhs)`
inline constexpr auto sub = detail::partial_n<2>(std::minus<>{});
/// `mul(L lhs, R rhs) -> decltype(lhs * rhs)`
inline constexpr auto mul = detail::partial_n<2>(std::multiplies<>{});
/// `div(L lhs, R rhs) -> decltype(lhs / rhs)`
inline constexpr auto div = detail::partial_n<2>(std::divides<>{});
/// `mod(L lhs, R rhs) -> decltype(lhs % rhs)`
inline constexpr auto mod = detail::partial_n<2>(std::modulus<>{});
/// `neg(T value) -> decltype(-value)`
inline constexpr auto neg = detail::partial_n<1>(std::negate<>{});

// Explicit lambda implementations make these operations usable in constant expressions.
/// `inc(T value) -> decltype(value + 1)`
inline constexpr auto inc = detail::partial_n<1>([](const auto& x) { return x + 1; });
/// `square(T value) -> decltype(value * value)`
inline constexpr auto square = detail::partial_n<1>([](const auto& x) { return x * x; });

} // namespace cpfp
