#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace cpfp::detail {

// A terminal owns its bound arguments and evaluates only once all N arguments are supplied.
// The final argument can also be passed through operator|.
template <std::size_t N, class F, class... Bound> struct terminal_partial {
    static_assert(sizeof...(Bound) < N);

    F function;
    std::tuple<Bound...> bound;

  private:
    template <class Self, class... Args>
    static constexpr decltype(auto) apply(Self&& self, Args&&... args) {
        if constexpr (sizeof...(Bound) + sizeof...(Args) == N) {
            return std::apply(
                [&](auto&&... saved) -> decltype(auto) {
                    return std::invoke(std::forward_like<Self>(self.function),
                                       std::forward<decltype(saved)>(saved)...,
                                       std::forward<Args>(args)...);
                },
                std::forward_like<Self>(self.bound));
        } else {
            return std::apply(
                [&](auto&&... saved) {
                    return terminal_partial<N, F, Bound..., std::decay_t<Args>...>{
                        std::forward_like<Self>(self.function),
                        std::tuple<Bound..., std::decay_t<Args>...>{
                            std::forward<decltype(saved)>(saved)..., std::forward<Args>(args)...}};
                },
                std::forward_like<Self>(self.bound));
        }
    }

  public:
    template <class... Args>
        requires(sizeof...(Args) > 0 && sizeof...(Bound) + sizeof...(Args) <= N &&
                 ((sizeof...(Bound) + sizeof...(Args) == N &&
                   std::invocable<F&, Bound&..., Args && ...>) ||
                  (sizeof...(Bound) + sizeof...(Args) < N && std::constructible_from<F, F&> &&
                   (std::constructible_from<Bound, Bound&> && ...) &&
                   (std::constructible_from<std::decay_t<Args>, Args &&> && ...))))
    constexpr decltype(auto) operator()(Args&&... args) & {
        return apply(*this, std::forward<Args>(args)...);
    }

    template <class... Args>
        requires(sizeof...(Args) > 0 && sizeof...(Bound) + sizeof...(Args) <= N &&
                 ((sizeof...(Bound) + sizeof...(Args) == N &&
                   std::invocable<const F&, const Bound&..., Args && ...>) ||
                  (sizeof...(Bound) + sizeof...(Args) < N && std::constructible_from<F, const F&> &&
                   (std::constructible_from<Bound, const Bound&> && ...) &&
                   (std::constructible_from<std::decay_t<Args>, Args &&> && ...))))
    constexpr decltype(auto) operator()(Args&&... args) const& {
        return apply(*this, std::forward<Args>(args)...);
    }

    template <class... Args>
        requires(sizeof...(Args) > 0 && sizeof...(Bound) + sizeof...(Args) <= N &&
                 ((sizeof...(Bound) + sizeof...(Args) == N &&
                   std::invocable<F &&, Bound && ..., Args && ...>) ||
                  (sizeof...(Bound) + sizeof...(Args) < N && std::constructible_from<F, F &&> &&
                   (std::constructible_from<Bound, Bound &&> && ...) &&
                   (std::constructible_from<std::decay_t<Args>, Args &&> && ...))))
    constexpr decltype(auto) operator()(Args&&... args) && {
        return apply(std::move(*this), std::forward<Args>(args)...);
    }

    template <class R>
        requires(sizeof...(Bound) + 1 == N && std::invocable<F&, Bound&..., R &&>)
    friend constexpr decltype(auto) operator|(R&& range, terminal_partial& terminal) {
        return terminal(std::forward<R>(range));
    }

    template <class R>
        requires(sizeof...(Bound) + 1 == N && std::invocable<const F&, const Bound&..., R &&>)
    friend constexpr decltype(auto) operator|(R&& range, const terminal_partial& terminal) {
        return terminal(std::forward<R>(range));
    }

    template <class R>
        requires(sizeof...(Bound) + 1 == N && std::invocable<F &&, Bound && ..., R &&>)
    friend constexpr decltype(auto) operator|(R&& range, terminal_partial&& terminal) {
        return std::move(terminal)(std::forward<R>(range));
    }
};

template <std::size_t N, class F> constexpr auto terminal_partial_n(F function) {
    static_assert(N > 0);
    return terminal_partial<N, F>{std::move(function), {}};
}

} // namespace cpfp::detail
