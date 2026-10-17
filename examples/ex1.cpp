#include "cpfp.hpp"

#include <boost/hof/flow.hpp>

#include <iostream>
#include <ranges>
#include <vector>

struct person {
    int age;
};

int main() {
    namespace v = std::views;

    const auto sum_even_squares =
        boost::hof::flow(v::filter(cpfp::even), v::transform(cpfp::square), cpfp::sum);

    std::vector xs{1, 2, 3, 4, 5, 6};
    std::cout << sum_even_squares(xs) << '\n'; // 56

    const auto younger = cpfp::on(cpfp::lt, &person::age);
    std::cout << std::boolalpha << younger(person{18}, person{26}) << '\n';

    const auto minus_10 = cpfp::flip(cpfp::sub)(10);
    std::cout << minus_10(3) << '\n'; // -7
}
