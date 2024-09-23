#ifndef CACHEHOUND_CONCEPTS_EXTENDED_MEMORY_REGION_RANGE_HPP
#define CACHEHOUND_CONCEPTS_EXTENDED_MEMORY_REGION_RANGE_HPP

#include <ranges>

#include "./extended_memory_region.hpp"
#include "./memory_region_range.hpp"

namespace cachehound {

template<typename R>
concept extended_memory_region_range = memory_region_range<R> and extended_memory_region<std::ranges::range_value_t<R>>;

}

#endif /* CACHEHOUND_CONCEPTS_EXTENDED_MEMORY_REGION_RANGE_HPP */
