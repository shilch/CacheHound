#ifndef CACHEHOUND_CONCEPTS_MEMORY_REGION_HPP
#define CACHEHOUND_CONCEPTS_MEMORY_REGION_HPP

#include <concepts>
#include <cstdint>
#include <cstddef>

namespace cachehound {

template<typename R>
concept memory_region = requires(const R& const_region) {
    { const_region.base() } noexcept -> std::same_as<std::uintptr_t>;
    { const_region.size() } noexcept -> std::same_as<std::size_t>;
};


}

#endif /* CACHEHOUND_CONCEPTS_MEMORY_REGION_HPP */
