#pragma once

#include <boost/hof/compose.hpp>
#include <boost/hof/limit.hpp>
#include <boost/hof/partial.hpp>

#include <cstddef>
#include <type_traits>

namespace cpfp::detail {

// Apply a parameter limit, then turn the function into a partial application.
template <std::size_t N>
inline constexpr auto partial_n = boost::hof::compose(
    boost::hof::partial, boost::hof::limit(std::integral_constant<std::size_t, N>{}));

} // namespace cpfp::detail
