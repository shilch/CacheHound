#ifndef CACHEHOUND_UTIL_SET_CYCLING_ADDRESS_DISTRIBUTION_HPP
#define CACHEHOUND_UTIL_SET_CYCLING_ADDRESS_DISTRIBUTION_HPP

#include "../concepts/placement_policy.hpp"
#include "../concepts/memory.hpp"
#include "./uniform_address_distribution.hpp"

#include <functional>
#include <random>

namespace cachehound {

class set_cycling_address_distribution {
    using index_function = std::size_t(std::uintptr_t);

    const std::size_t sets_;
    const std::size_t starting_set_;

    uniform_address_distribution base_distribution_;
    std::size_t next_set_;
    std::function<index_function> index_function_;

public:
    set_cycling_address_distribution(
        memory auto& memory,
        placement_policy auto&& policy,
        std::size_t starting_set = 0);

    set_cycling_address_distribution(memory auto& memory, std::size_t starting_set = 0);

    template<std::uniform_random_bit_generator URBG>
    std::uintptr_t operator()(URBG& urbg);

    void reset();
};

}

#include "./impl/set_cycling_address_distribution.hpp"

#endif /* CACHEHOUND_UTIL_SET_CYCLING_ADDRESS_DISTRIBUTION_HPP */
