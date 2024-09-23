#ifndef CACHEHOUND_POLICIES_PLACEMENT_GENERIC_PLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_PLACEMENT_GENERIC_PLACEMENT_POLICY_HPP

#include "../../concepts/placement_policy.hpp"

#include <bit>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <functional>

namespace cachehound {

class generic_placement_policy {
    using signature = std::size_t(std::uintptr_t);

    std::size_t sets_;
    std::function<signature> func_;

public:
    template<typename Func>
    requires requires(Func f, std::uintptr_t address) {
        { f(address) } -> std::same_as<std::size_t>;
    }
    generic_placement_policy(std::size_t sets, Func&& f)
        : sets_(sets)
        , func_(std::forward<Func>(f)) {}

    std::uint8_t index_bits() const noexcept {
        assert(sets_ != 0 && (sets_ & (sets_ - 1)) == 0);
        return std::countr_zero(sets_);
    }

    std::size_t operator()(std::uintptr_t address) const noexcept {
        return func_(address);
    }
};
static_assert(placement_policy<generic_placement_policy>);

}

#endif /* CACHEHOUND_POLICIES_PLACEMENT_GENERIC_PLACEMENT_POLICY_HPP */
