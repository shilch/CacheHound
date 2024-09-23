#ifndef CACHEHOUND_UTIL_CONCATENATED_ADDRESS_DISTRIBUTION_HPP
#define CACHEHOUND_UTIL_CONCATENATED_ADDRESS_DISTRIBUTION_HPP

#include "../concepts/address_distribution.hpp"
#include "./uniform_address_distribution.hpp"

#include <random>

namespace cachehound {

template<address_distribution D1, address_distribution D2 = D1>
class concatenated_address_distribution {
    const std::size_t draws_;

    std::size_t current_draws_;
    D1 d1_;
    D2 d2_;

public:
    concatenated_address_distribution(D1&& d1, std::size_t draws, D2&& d2);

    template<std::uniform_random_bit_generator URBG>
    std::uintptr_t operator()(URBG&);

    void reset();
};
static_assert(address_distribution<concatenated_address_distribution<uniform_address_distribution>>);

}

#include "./impl/concatenated_address_distribution.hpp"

#endif /* CACHEHOUND_UTIL_CONCATENATED_ADDRESS_DISTRIBUTION_HPP */
