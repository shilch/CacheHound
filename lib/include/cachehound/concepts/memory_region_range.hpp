#ifndef CACHEHOUND_CONCEPTS_MEMORY_REGION_RANGE_HPP
#define CACHEHOUND_CONCEPTS_MEMORY_REGION_RANGE_HPP

#include <ranges>

#include "./memory_region.hpp"

namespace cachehound {

template<typename R>
concept memory_region_range = std::ranges::random_access_range<R> and memory_region<std::ranges::range_value_t<R>>;

}

#endif /* CACHEHOUND_CONCEPTS_MEMORY_REGION_RANGE_HPP */
