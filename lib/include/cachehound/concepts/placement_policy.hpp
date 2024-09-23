#ifndef CACHEHOUND_CONCEPTS_PLACEMENT_POLICY_HPP
#define CACHEHOUND_CONCEPTS_PLACEMENT_POLICY_HPP

#include <cstdint>
#include <cstddef>
#include <concepts>

namespace cachehound {

template<typename P>
concept placement_policy = requires(const P const_policy, std::uintptr_t address) {
    { const_policy.index_bits() } noexcept -> std::same_as<std::uint8_t>;
    { const_policy(address) } noexcept -> std::same_as<std::size_t>;
};

}

#endif /* CACHEHOUND_CONCEPTS_PLACEMENT_POLICY_HPP */
