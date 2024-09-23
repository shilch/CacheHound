#ifndef CACHEHOUND_CONCEPTS_SEQUENCE_HPP
#define CACHEHOUND_CONCEPTS_SEQUENCE_HPP

#include <concepts>
#include <ranges>

namespace cachehound {

template<typename R>
concept sequence = std::ranges::input_range<R> and std::same_as<std::ranges::range_value_t<R>, std::size_t>;

}

#endif /* CACHEHOUND_CONCEPTS_SEQUENCE_HPP */
