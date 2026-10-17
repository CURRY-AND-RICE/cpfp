#pragma once

#include <functional>
#include <utility>

namespace cpfp {

// Short-circuit predicate combinators; unlike Lambda2's && and || expressions.
/// `all_pass(P... predicates) -> (T value) -> bool`
template <class... Predicates> constexpr auto all_pass(Predicates&&... predicates) {
    return [... ps = std::forward<Predicates>(predicates)](auto&& x) {
        // x is reused without forwarding it multiple times.
        return (... && static_cast<bool>(std::invoke(ps, x)));
    };
}

/// `any_pass(P... predicates) -> (T value) -> bool`
template <class... Predicates> constexpr auto any_pass(Predicates&&... predicates) {
    return [... ps = std::forward<Predicates>(predicates)](auto&& x) {
        return (... || static_cast<bool>(std::invoke(ps, x)));
    };
}

} // namespace cpfp
