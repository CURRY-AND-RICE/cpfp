#pragma once

#include "cpfp/detail/partial_n.hpp"

#include <boost/hof/limit.hpp>
#include <boost/hof/pipable.hpp>

#include <concepts>
#include <cstddef>
#include <functional>
#include <utility>

namespace cpfp::detail {

// Keep Boost.HOF's argument storage, adding piping when only the range remains.
template <std::size_t N, class F> struct terminal_partial {
    F function;

    template <class... Args>
        requires(sizeof...(Args) <= N && std::invocable<const F&, Args...>)
    constexpr decltype(auto) operator()(Args&&... args) const {
        if constexpr (sizeof...(Args) == N) {
            return std::invoke(function, std::forward<Args>(args)...);
        } else {
            auto next = std::invoke(function, std::forward<Args>(args)...);
            if constexpr (N - sizeof...(Args) == 1) {
                // Bound partial adaptors still carry the original parameter limit.
                return boost::hof::pipable(boost::hof::limit_c<1>(std::move(next)));
            } else {
                return terminal_partial<N - sizeof...(Args), decltype(next)>{std::move(next)};
            }
        }
    }
};

template <std::size_t N, class F> constexpr auto terminal_partial_n(F function) {
    static_assert(N > 0);
    auto partial = partial_n<N>(std::move(function));
    if constexpr (N == 1) {
        return boost::hof::pipable(boost::hof::limit_c<1>(std::move(partial)));
    } else {
        return terminal_partial<N, decltype(partial)>{std::move(partial)};
    }
}

} // namespace cpfp::detail
