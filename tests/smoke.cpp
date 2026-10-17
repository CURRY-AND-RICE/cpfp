#include "cpfp.hpp"

#include <boost/hof/flow.hpp>

#include <concepts>
#include <cstdlib>
#include <functional>
#include <ranges>
#include <vector>

static_assert(cpfp::add(2, 3) == 5);
static_assert(cpfp::add(2)(3) == 5);
static_assert(cpfp::sub(10)(3) == 7);
static_assert(cpfp::square(4) == 16);
static_assert(cpfp::inc(9) == 10);
static_assert(cpfp::even(4));
static_assert(cpfp::odd(3));
static_assert(cpfp::between(1, 10)(5));
static_assert([] {
    int xs[]{1, 2, 3};
    return (xs | cpfp::foldl(cpfp::add)(0)) == 6;
}());
static_assert(!std::invocable<decltype(cpfp::foldl), std::plus<>, int, std::vector<int>&, int>);
static_assert(!std::invocable<decltype(cpfp::foldl(cpfp::add)), int, std::vector<int>&, int>);
static_assert(!std::invocable<decltype(cpfp::foldl(cpfp::add)(0)), std::vector<int>&, int>);

struct person {
    int age;
    int get_age() const { return age; }
};

void check(bool condition) {
    if (!condition) {
        std::abort();
    }
}

int main() {
    namespace v = std::views;

    check(cpfp::even(4));
    check(cpfp::odd(3));
    check(cpfp::flip(cpfp::sub)(10)(3) == -7);

    auto younger = cpfp::on(cpfp::lt, &person::age);
    check(younger(person{18}, person{27}));
    check(cpfp::on(cpfp::lt, &person::get_age)(person{18}, person{27}));

    auto valid = cpfp::all_pass([](int x) { return x != 0; }, [](int x) { return 10 / x > 2; });
    check(!valid(0));
    check(valid(2));
    check(cpfp::any_pass([](int x) { return x == 1; }, [](int x) { return x == 2; })(2));

    int threshold = 10;
    auto copied_threshold = cpfp::lt(threshold);
    auto referenced_threshold = cpfp::lt(std::cref(threshold));
    threshold = 20;
    check(copied_threshold(15));
    check(!referenced_threshold(15));

    std::vector expected{1, 2};
    auto copied_vector = cpfp::eq(expected);
    auto referenced_vector = cpfp::eq(std::cref(expected));
    expected.push_back(3);
    check(!copied_vector(expected));
    check(referenced_vector(expected));

    std::vector xs{1, 2, 3, 4, 5, 6};
    auto evaluate = boost::hof::flow(v::filter(cpfp::even), v::transform(cpfp::square), cpfp::sum);
    check(evaluate(xs) == 56);
    check(cpfp::foldl(cpfp::add, 0, xs) == 21);
    check(cpfp::foldl(cpfp::add, 0)(xs) == 21);
    check(cpfp::foldl(cpfp::add)(0, xs) == 21);
    check(cpfp::foldl(cpfp::add)(0)(xs) == 21);
    check((xs | cpfp::sum) == 21);
    check((xs | cpfp::foldl(cpfp::add, 0)) == 21);
    check((xs | cpfp::foldl(cpfp::add)(0)) == 21);
    check((xs | v::filter(cpfp::even) | v::transform(cpfp::square) | cpfp::sum) == 56);
    check((xs | v::filter(cpfp::even) | cpfp::foldl(cpfp::mul)(1)) == 48);
    check((std::vector{1, 2, 3} | cpfp::foldl(cpfp::sub)(100)) == 94);

    auto fold_evaluate = boost::hof::flow(v::filter(cpfp::even), cpfp::foldl(cpfp::add)(0));
    check(fold_evaluate(xs) == 12);

    // Each evaluation starts from the saved accumulator, even after it was moved into fold_left.
    std::vector initial{10};
    const auto collect = cpfp::foldl([](std::vector<int> acc, int x) {
        acc.push_back(x);
        return acc;
    })(initial);
    initial.push_back(99);
    const std::vector collected{10, 1, 2, 3, 4, 5, 6};
    check((xs | collect) == collected);
    check((xs | collect) == collected);

    std::vector<int> empty;
    check((empty | cpfp::sum) == 0);
    check((empty | cpfp::foldl(cpfp::add)(7)) == 7);
}
