#ifndef CACHEHOUND_CONCEPTS_ADDRESS_RANGE_HPP
#define CACHEHOUND_CONCEPTS_ADDRESS_RANGE_HPP

#include <concepts>
#include <ranges>

namespace cachehound {

template<typename R>
concept address_range = std::ranges::forward_range<R> and std::same_as<std::ranges::range_value_t<R>, std::uintptr_t>;

}

#endif /* CACHEHOUND_CONCEPTS_ADDRESS_RANGE_HPP */
