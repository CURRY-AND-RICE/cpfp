#pragma once

#include "cpfp/detail/terminal_partial_n.hpp"

#include <algorithm>
#include <boost/hof/limit.hpp>
#include <boost/hof/pipable.hpp>
#include <functional>
#include <ranges>
#include <utility>

namespace cpfp {

// Arguments are ordered for pipeline use: foldl(operation)(initial)(range).
/// `foldl(F op, T init, R&& range) -> decltype(ranges::fold_left(range, init, op))`
inline constexpr auto foldl = detail::terminal_partial_n<3>(
    [](auto operation, auto initial, auto&& range)
        requires requires {
            std::ranges::fold_left(std::forward<decltype(range)>(range), std::move(initial),
                                   std::move(operation));
        }
    {
        return std::ranges::fold_left(std::forward<decltype(range)>(range), std::move(initial),
                                      std::move(operation));
    });

// Value-initialized accumulator of the range value type. Like Haskell's sum,
// this requires a meaningful additive identity for the type.
/// `sum(R&& range) -> decltype(ranges::fold_left(range, range_value_t<R>{}, plus{}))`
inline constexpr auto sum =
    boost::hof::pipable(boost::hof::limit_c<1>([]<std::ranges::input_range R>(R&& range) {
        using value_type = std::ranges::range_value_t<R>;
        return std::ranges::fold_left(std::forward<R>(range), value_type{}, std::plus<>{});
    }));

} // namespace cpfp
