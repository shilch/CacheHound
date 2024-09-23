#ifndef CACHEHOUND_CONCEPTS_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_CONCEPTS_REPLACEMENT_POLICY_HPP

#include <concepts>

namespace cachehound {

template<typename P>
concept replacement_policy = requires(P policy, const P const_policy, std::size_t way) {
    { const_policy.ways() } noexcept -> std::same_as<std::size_t>;
    { policy.hit(way) } -> std::same_as<void>;
    { policy.miss() } -> std::same_as<std::size_t>;
};

}

#endif /* CACHEHOUND_CONCEPTS_REPLACEMENT_POLICY_HPP */
