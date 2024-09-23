#ifndef CACHEHOUND_SIM_CONCEPTS_SET_ASSOCIATIVE_POLICY_HPP
#define CACHEHOUND_SIM_CONCEPTS_SET_ASSOCIATIVE_POLICY_HPP

#include <concepts>

namespace cachehound::sim {

template<typename P>
concept set_associative_policy = requires(P policy, const P& const_policy, std::uintptr_t address, std::size_t set_index, std::size_t way_index) {
    { const_policy.index_bits() } noexcept -> std::same_as<std::uint8_t>;
    { const_policy.ways() } noexcept -> std::same_as<std::size_t>;
    { const_policy.get_set(address) } noexcept -> std::same_as<std::size_t>;
    { policy.register_hit(set_index, way_index) } noexcept;
    { policy.register_miss(set_index) } noexcept -> std::same_as<std::size_t>;
};

}

#endif /* CACHEHOUND_SIM_CONCEPTS_SET_ASSOCIATIVE_POLICY_HPP */
