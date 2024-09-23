#ifndef CACHEHOUND_POLICIES_PLACEMENT_MODULAR_PLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_PLACEMENT_MODULAR_PLACEMENT_POLICY_HPP

#include <bit>
#include <cassert>

#include "../../concepts/placement_policy.hpp"

namespace cachehound {

class modular_placement_policy {
    const std::size_t offset_bits_;
    const std::size_t sets_;

public:
    modular_placement_policy(std::size_t offset_bits, std::size_t sets)
        : offset_bits_(offset_bits)
        , sets_(sets) {
            assert(sets != 0 && (sets & (sets - 1)) == 0);
        }

    std::uint8_t index_bits() const noexcept {
        return std::countr_zero(sets_);
    }

    std::size_t operator()(std::uintptr_t address) const noexcept {
        return (address >> offset_bits_) % sets_;
    }
};
static_assert(placement_policy<modular_placement_policy>);

}

#endif /* CACHEHOUND_POLICIES_PLACEMENT_MODULAR_PLACEMENT_POLICY_HPP */
