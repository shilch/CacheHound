#ifndef CACHEHOUND_UTIL_IMPL_SET_CYCLING_ADDRESS_DISTRIBUTION_HPP
#define CACHEHOUND_UTIL_IMPL_SET_CYCLING_ADDRESS_DISTRIBUTION_HPP

#include "../set_cycling_address_distribution.hpp"

#include "../../policies/placement/modular_placement_policy.hpp"

#include <utility>

cachehound::set_cycling_address_distribution::set_cycling_address_distribution(
    memory auto& memory,
    placement_policy auto&& placement_policy,
    std::size_t starting_set)
    : sets_(1 << placement_policy.index_bits())
    , starting_set_(starting_set)
    , base_distribution_(memory)
    , index_function_([p = std::forward<decltype(placement_policy)>(placement_policy)](std::uintptr_t address) mutable {
        return std::forward<decltype(placement_policy)>(p)(address);
    })
    , next_set_(starting_set)
{
}

cachehound::set_cycling_address_distribution::set_cycling_address_distribution(
    memory auto& memory,
    std::size_t starting_set)
    : set_cycling_address_distribution(memory, modular_placement_policy { memory.offset_bits(), std::size_t { 1 } << memory.index_bits(0) }, starting_set)
{
}

template <std::uniform_random_bit_generator URBG>
std::uintptr_t cachehound::set_cycling_address_distribution::operator()(URBG& urbg)
{
    std::uintptr_t address;
    while (index_function_(address = base_distribution_(urbg)) != next_set_)
        ;
    next_set_ = (next_set_ + 1) % sets_;
    return address;
}

void cachehound::set_cycling_address_distribution::reset()
{
    base_distribution_.reset();
    next_set_ = starting_set_;
}

#endif /* CACHEHOUND_UTIL_IMPL_SET_CYCLING_ADDRESS_DISTRIBUTION_HPP */
