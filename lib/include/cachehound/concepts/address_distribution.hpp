#ifndef CACHEHOUND_CONCEPTS_ADDRESS_DISTRIBUTION_HPP
#define CACHEHOUND_CONCEPTS_ADDRESS_DISTRIBUTION_HPP

#include <concepts>
#include <cstdint>
#include <random>

namespace cachehound {

template<typename D>
concept address_distribution = requires(D distribution, std::random_device urbg) {
    { distribution(urbg) } -> std::same_as<std::uintptr_t>;
    { distribution.reset() };
};

}

#endif /* CACHEHOUND_CONCEPTS_ADDRESS_DISTRIBUTION_HPP */
