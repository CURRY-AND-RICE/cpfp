#include "cpfp/fold.hpp"

#include <boost/hof/flow.hpp>

#include <concepts>
#include <cstdlib>
#include <functional>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace {

struct move_accumulate {
    std::unique_ptr<int> operator()(std::unique_ptr<int> acc, int x) const {
        *acc += x;
        return acc;
    }
};

void check(bool result) {
    if (!result) {
        std::abort();
    }
}

using move_terminal = decltype(cpfp::foldl(move_accumulate{}, std::make_unique<int>(10)));

static_assert(!std::invocable<decltype(cpfp::foldl)>);
static_assert(!std::invocable<decltype(cpfp::foldl(std::plus<>{}))>);
static_assert(!std::invocable<decltype(cpfp::foldl), std::plus<>, int, int>);
static_assert(!std::invocable<decltype(cpfp::foldl), std::plus<>, int, std::vector<int>&, int>);
static_assert(!std::invocable<decltype(cpfp::foldl(std::plus<>{}, 0)), int>);
static_assert(!std::invocable<decltype(cpfp::foldl(std::plus<>{}, 0)), std::vector<int>&, int>);
static_assert(!std::invocable<move_terminal&, std::vector<int>&>);
static_assert(!std::invocable<const move_terminal&, std::vector<int>&>);
static_assert(std::invocable<move_terminal&&, std::vector<int>&>);

constexpr auto unary = cpfp::detail::terminal_partial_n<1>([](int x) { return x + 1; });
static_assert(unary(2) == 3);
static_assert((2 | unary) == 3);
static_assert(!std::invocable<decltype(unary)>);
static_assert(!std::invocable<decltype(unary), int, int>);

static_assert([] {
    int xs[]{1, 2, 3};
    return (xs | cpfp::foldl(std::plus<>{}, 0)) == 6;
}());

} // namespace

int main() {
    std::vector xs{1, 2, 3};
    const auto op = std::plus<>{};

    // Direct, partially applied, curried, pipe, and flow call styles remain available.
    check(cpfp::foldl(op, 0, xs) == 6);
    check(cpfp::foldl(op, 0)(xs) == 6);
    check(cpfp::foldl(op)(0, xs) == 6);
    check(cpfp::foldl(op)(0)(xs) == 6);
    check((xs | cpfp::foldl(op, 0)) == 6);
    check((xs | cpfp::foldl(op)(0)) == 6);
    check((std::vector{1, 2, 3} | cpfp::foldl(op, 0)) == 6);
    const auto reusable = cpfp::foldl(op, 0);
    check((xs | reusable) == 6);
    check((xs | reusable) == 6);
    check(boost::hof::flow(std::views::transform([](int x) { return x * 2; }),
                           cpfp::foldl(op, 0))(xs) == 12);

    // A bound lvalue remains reusable: each fold starts from a copy of its saved init.
    const auto collect = cpfp::foldl(
        [](std::vector<int> acc, int x) {
            acc.push_back(x);
            return acc;
        },
        std::vector{10});
    check((xs | collect) == std::vector<int>({10, 1, 2, 3}));
    check((xs | collect) == std::vector<int>({10, 1, 2, 3}));

    // Move-only accumulator: consuming an rvalue closure forwards its captured init.
    auto consume = cpfp::foldl(move_accumulate{}, std::make_unique<int>(10));
    check(*std::move(consume)(xs) == 16);
    check(*(xs | cpfp::foldl(move_accumulate{}, std::make_unique<int>(10))) == 16);
    check(*(xs | cpfp::foldl(move_accumulate{})(std::make_unique<int>(10))) == 16);
    check(*cpfp::foldl(move_accumulate{}, std::make_unique<int>(10), xs) == 16);

    // Move-only callable: intermediate partial applications also forward ownership.
    auto op_partial = cpfp::detail::terminal_partial_n<3>(
        [increment = std::make_unique<int>(1)](int multiplier, int initial,
                                               const std::vector<int>& range) {
            for (int value : range) {
                initial += multiplier * *increment * value;
            }
            return initial;
        });
    static_assert(!std::invocable<decltype(op_partial)&, int>);
    static_assert(std::invocable<decltype(op_partial)&&, int>);
    auto op_next = std::move(op_partial)(2);
    static_assert(!std::invocable<decltype(op_next)&, int>);
    auto op_terminal = std::move(op_next)(0);
    check((xs | std::move(op_terminal)) == 12);

    // Non-const lvalue terminal callbacks may maintain state.
    auto stateful = cpfp::detail::terminal_partial_n<1>(
        [counter = 0](int value) mutable { return value + ++counter; });
    check(stateful(10) == 11);
    check(stateful(10) == 12);
}
