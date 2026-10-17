#pragma once

#include "cpfp/detail/partial_n.hpp"

#include <boost/hof/flip.hpp>

#include <functional>
#include <utility>

namespace cpfp {

// Unlike boost::hof::flip, this is partially applicable as flip(f)(rhs)(lhs).
/// `flip(F function) -> (R rhs, L lhs) -> decltype(function(lhs, rhs))`
template <class F> constexpr auto flip(F&& f) {
    return detail::partial_n<2>(boost::hof::flip(std::forward<F>(f)));
}

// Haskell-like (on): on(f, projection)(x, y) = f(projection(x), projection(y)).
// std::invoke handles pointers to data members and member functions.
/// `on(F function, P projection) -> (X x, Y y) -> decltype(function(projection(x), projection(y)))`
inline constexpr auto on = detail::partial_n<2>([](auto f, auto projection) {
    return detail::partial_n<2>([f = std::move(f), projection = std::move(projection)](
                                    auto&& x, auto&& y) -> decltype(auto) {
        return std::invoke(f, std::invoke(projection, std::forward<decltype(x)>(x)),
                           std::invoke(projection, std::forward<decltype(y)>(y)));
    });
});

} // namespace cpfp
