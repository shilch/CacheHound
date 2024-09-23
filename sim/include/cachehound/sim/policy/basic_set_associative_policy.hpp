#ifndef CACHEHOUND_SIM_POLICY_BASIC_SET_ASSOCIATIVE_POLICY_HPP
#define CACHEHOUND_SIM_POLICY_BASIC_SET_ASSOCIATIVE_POLICY_HPP

#include <cstddef>
#include <vector>

#include <cachehound/cachehound.hpp>

#include "../concepts/set_associative_policy.hpp"

namespace cachehound::sim {

class basic_set_associative_policy {
    const any_placement_policy placement_;
    const any_replacement_policy replacement_template_;
    std::vector<any_replacement_policy> replacement_;

public:
    template<placement_policy PlacementPolicy, replacement_policy ReplacementPolicy>
    basic_set_associative_policy(
        std::uint8_t index_bits,
        PlacementPolicy placement_policy,
        ReplacementPolicy replacement_policy)
        : placement_(std::move(placement_policy))
        , replacement_template_(std::move(replacement_policy))
        , replacement_(1 << index_bits, replacement_template_) {}

    std::uint8_t index_bits() const noexcept {
        return placement_.index_bits();
    }

    std::size_t ways() const noexcept {
        return replacement_template_.ways();
    }

    std::size_t get_set(auto address) const noexcept {
        return placement_(address);
    }

    void register_hit(std::size_t set_index, std::size_t way_index) noexcept {
        replacement_[set_index].hit(way_index);
    }

    std::size_t register_miss(std::size_t set_index) noexcept {
        return replacement_[set_index].miss();
    }
};
static_assert(set_associative_policy<basic_set_associative_policy>);

}

#endif /* CACHEHOUND_SIM_POLICY_BASIC_SET_ASSOCIATIVE_POLICY_HPP */
