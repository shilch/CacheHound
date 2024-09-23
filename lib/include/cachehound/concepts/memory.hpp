#ifndef CACHEHOUND_CONCEPTS_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_MEMORY_HPP

#include <type_traits>

#include "./memory_region.hpp"
#include "./memory_region_range.hpp"

namespace cachehound {

template<typename M>
concept memory = memory_region<typename std::remove_cvref_t<M>::region_type> and requires(M& memory, const M& const_memory, std::uintptr_t address) {
    { memory.access(address) };
    { const_memory.regions() } noexcept -> memory_region_range;
    { const_memory.offset_bits() } noexcept -> std::same_as<std::uint8_t>;
};

}

#endif /* CACHEHOUND_CONCEPTS_MEMORY_HPP */
