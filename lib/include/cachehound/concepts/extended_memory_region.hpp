#ifndef CACHEHOUND_CONCEPTS_EXTENDED_MEMORY_REGION_HPP
#define CACHEHOUND_CONCEPTS_EXTENDED_MEMORY_REGION_HPP

#include <concepts>
#include <cstdint>
#include <cstddef>

#include "./memory_region.hpp"

namespace cachehound {

template<typename R>
concept extended_memory_region = memory_region<R> and requires(const R& const_region) {
    { const_region.physical_base() } noexcept -> std::same_as<std::uintptr_t>;
};


}

#endif /* CACHEHOUND_CONCEPTS_EXTENDED_MEMORY_REGION_HPP */
