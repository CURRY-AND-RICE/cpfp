#include "cpfp.hpp"

#include <boost/hof/flow.hpp>

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
    check(cpfp::foldl(cpfp::add, 0)(xs) == 21);
    check(cpfp::foldl(cpfp::add)(0)(xs) == 21);
}
