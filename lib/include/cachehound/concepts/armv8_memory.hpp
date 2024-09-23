#ifndef CACHEHOUND_CONCEPTS_ARMV8_MEMORY_HPP
#define CACHEHOUND_CONCEPTS_ARMV8_MEMORY_HPP

#include <concepts>

#include "./memory.hpp"

namespace cachehound {

template<typename M>
concept armv8_memory = memory<M> and requires(M& memory, unsigned level, std::size_t set, std::size_t way) {
    { memory.cisw(level, set, way) } -> std::same_as<void>;
    { memory.csw(level, set, way) } -> std::same_as<void>;
    { memory.isw(level, set, way) } -> std::same_as<void>;
};

}

#endif /* CACHEHOUND_CONCEPTS_ARMV8_MEMORY_HPP */
