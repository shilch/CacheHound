#ifndef CACHEHOUND_UTIL_UNIFORM_ADDRESS_DISTRIBUTION_HPP
#define CACHEHOUND_UTIL_UNIFORM_ADDRESS_DISTRIBUTION_HPP

#include <cassert>
#include <cstdint>
#include <random>

#include "../concepts/memory.hpp"
#include "../concepts/address_distribution.hpp"

namespace cachehound {

/**
 * @brief A uniform distribution of valid memory addresses
 */
class uniform_address_distribution {
    struct distributions {
        std::discrete_distribution<std::uintptr_t> regions;
        std::vector<std::uniform_int_distribution<std::uintptr_t>> addresses;
    };

    const std::uint8_t alignment_;
    const std::uintptr_t offset_;
    distributions distributions_;

    static distributions derive_distributions(const memory auto& memory, std::uint8_t alignment);

public:
    using result_type = std::uintptr_t;

    /**
     * @brief Construct a new uniform address distribution
     * 
     * @param memory The memory for which the addresses should be generated.
     * @param alignment The number of lower bits that should be zero,
     * e.g. pass 6 for 64-byte aligned addresses (i. e., align by cache line on Intel)
     * @param offset The offset to add on top of the final address, must fit into (alignment-1) bits
     */
    uniform_address_distribution(const memory auto& memory, std::uint8_t alignment, std::uintptr_t offset = 0);

    uniform_address_distribution(const memory auto& memory);

    void reset();

    std::uintptr_t operator()(std::uniform_random_bit_generator auto& g) noexcept(noexcept(distributions_.regions(g)) && noexcept(distributions_.addresses.front()(g)));
};
static_assert(address_distribution<uniform_address_distribution>);

}

#include "./impl/uniform_address_distribution.hpp"

#if defined(CACHEHOUND_HEADER_ONLY)
#include "./impl/uniform_address_distribution.ipp"
#endif

#endif /* CACHEHOUND_UTIL_UNIFORM_ADDRESS_DISTRIBUTION_HPP */
