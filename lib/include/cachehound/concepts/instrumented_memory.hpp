#ifndef CACHEHOUND_CONCEPTS_INSTRUMENTED_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_INSTRUMENTED_MEMORY_HPP

#include <cstdint>
#include <concepts>

#include "./memory.hpp"

namespace cachehound {

template<typename M>
concept instrumented_memory = memory<M>
    and requires(M& memory, const M& const_memory, std::uintptr_t address, unsigned level) {
    // The access(uintptr_t) member function accesses the memory at the
    // specified address and returns the latency in terms of levels
    // starting at 0.
    // 0 implies L1, 1 implies L2, ...
    { memory.instrumented_access(address) } -> std::same_as<unsigned>;

    { const_memory.levels() } noexcept -> std::same_as<unsigned>;
    { const_memory.index_bits(level) } noexcept -> std::same_as<std::uint8_t>;
    { const_memory.ways(level) } noexcept -> std::same_as<std::size_t>;
};

}

#endif /* CACHEHOUND_CONCEPTS_INSTRUMENTED_MEMORY_HPP */
