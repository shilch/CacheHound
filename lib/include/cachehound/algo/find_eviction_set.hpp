#ifndef CACHEHOUND_ALGO_FIND_EVICTION_SET_HPP
#define CACHEHOUND_ALGO_FIND_EVICTION_SET_HPP

#include <cstddef>
#include <cstdint>
#include <unordered_set>
#include <vector>

#include "./is_eviction_set.hpp"
#include "../concepts/instrumented_memory.hpp"
#include "../concepts/address_distribution.hpp"
#include "../util/uniform_address_distribution.hpp"

namespace cachehound {

/**
 * @brief Finds an eviction set of arbitrary size
 * 
 * @param is_eviction_set The function to check whether a set constitutes an eviction set for the given target
 * @param memory The memory to perform the accesses on
 * @param target The target address for that eviction set
 * @param offset_bits The number of bits by which cache lines are aligned, typically 6 bits for 64-byte cache lines
 * @param urbg Uniform random bit generator supplied to distribution
 * @param distribution An address distribution used for sourcing addresses
 * @return std::vector<std::uintptr_t> A valid eviction set for the target address
 */
[[nodiscard]] std::vector<std::uintptr_t> find_eviction_set(
    is_eviction_set auto&& is_eviction_set,
    instrumented_memory auto& memory,
    std::uintptr_t target,
    std::uniform_random_bit_generator auto& urbg,
    address_distribution auto& distribution
);

[[nodiscard]] std::vector<std::uintptr_t> find_eviction_set(
    is_eviction_set auto&& is_eviction_set,
    instrumented_memory auto& memory,
    std::uintptr_t target,
    std::uniform_random_bit_generator auto& urbg
);

}

#include "./impl/find_eviction_set.hpp"

#endif /* CACHEHOUND_ALGO_FIND_EVICTION_SET_HPP */
