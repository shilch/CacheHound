#ifndef CACHEHOUND_POLICIES_REPLACEMENT_LINEAR_INIT_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_REPLACEMENT_LINEAR_INIT_REPLACEMENT_POLICY_HPP

#include <vector>

#include "../../concepts/replacement_policy.hpp"

namespace cachehound {

template<replacement_policy NestedReplacementPolicy>
class linear_init_replacement_policy  {
    NestedReplacementPolicy nested_;
    std::size_t dirty_idx_ = 0;

public:
    linear_init_replacement_policy(NestedReplacementPolicy nested)
        : nested_(std::move(nested)) {}

    std::size_t ways() const noexcept {
        return nested_.ways();
    }

    void hit(std::size_t way) noexcept(noexcept(nested_.hit(way))) {
        assert(way < dirty_idx_);
        nested_.hit(way);
    }

    std::size_t miss() noexcept(noexcept(nested_.miss())) {
        if(dirty_idx_ < ways()) {
            std::size_t way = dirty_idx_++;
            nested_.hit(way);
            return way;
        } else {
            return nested_.miss();
        }
    }
};

}

#endif /* CACHEHOUND_POLICIES_REPLACEMENT_LINEAR_INIT_REPLACEMENT_POLICY_HPP */
