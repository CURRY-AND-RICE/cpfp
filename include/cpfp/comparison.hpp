#pragma once

#include "cpfp/detail/partial_n.hpp"

#include <functional>

namespace cpfp {

/// `eq(L lhs, R rhs) -> decltype(lhs == rhs)`
inline constexpr auto eq = detail::partial_n<2>(std::equal_to<>{});
/// `ne(L lhs, R rhs) -> decltype(lhs != rhs)`
inline constexpr auto ne = detail::partial_n<2>(std::not_equal_to<>{});
/// `lt(L lhs, R rhs) -> decltype(lhs < rhs)`
inline constexpr auto lt = detail::partial_n<2>(std::less<>{});
/// `le(L lhs, R rhs) -> decltype(lhs <= rhs)`
inline constexpr auto le = detail::partial_n<2>(std::less_equal<>{});
/// `gt(L lhs, R rhs) -> decltype(lhs > rhs)`
inline constexpr auto gt = detail::partial_n<2>(std::greater<>{});
/// `ge(L lhs, R rhs) -> decltype(lhs >= rhs)`
inline constexpr auto ge = detail::partial_n<2>(std::greater_equal<>{});

/// `even(T value) -> decltype(value % 2 == 0)`
inline constexpr auto even = detail::partial_n<1>([](const auto& value) { return value % 2 == 0; });
/// `odd(T value) -> decltype(value % 2 != 0)`
inline constexpr auto odd = detail::partial_n<1>([](const auto& value) { return value % 2 != 0; });

// Configuration arguments first; the value being inspected last.
/// `between(L lo, H hi, T value) -> decltype(lo <= value && value <= hi)`
inline constexpr auto between = detail::partial_n<3>(
    [](const auto& lo, const auto& hi, const auto& value) { return lo <= value && value <= hi; });

} // namespace cpfp
