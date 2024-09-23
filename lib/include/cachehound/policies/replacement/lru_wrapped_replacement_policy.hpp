#ifndef CACHEHOUND_POLICIES_REPLACEMENT_LRU_WRAPPED_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_REPLACEMENT_LRU_WRAPPED_REPLACEMENT_POLICY_HPP

#include <vector>

#include "./lru_replacement_policy.hpp"
#include "./plru_replacement_policy.hpp"

#include "../../concepts/replacement_policy.hpp"

namespace cachehound {

template<replacement_policy NestedReplacementPolicy>
class lru_wrapped_replacement_policy : public lru_replacement_policy {
    std::vector<NestedReplacementPolicy> nested_;
    std::size_t dirty_idx_ = 0;

    std::size_t nested_ways() const {
        return nested_.front().ways();
    }

public:
    lru_wrapped_replacement_policy(std::size_t lru_ways, const NestedReplacementPolicy& nested)
        : lru_replacement_policy(lru_ways) {
            nested_.reserve(lru_ways);
            for(std::size_t i = 0; i < lru_ways; i++) {
                nested_.push_back(nested);
                assert(nested_.front().ways() == nested_.back().ways());
            }
        }

    std::size_t ways() const noexcept {
        return lru_replacement_policy::ways() * nested_ways();
    }

    void hit(std::size_t way) noexcept(noexcept(lru_replacement_policy::hit(way))) {
        auto lru_way = way / nested_ways();
        auto nested_way = way % nested_ways();
        lru_replacement_policy::hit(lru_way);
        nested_[lru_way].hit(nested_way);
    }

    std::size_t miss() noexcept(noexcept(lru_replacement_policy::miss())) {
        if(dirty_idx_ < ways()) {
            std::size_t way = dirty_idx_++;
            hit(way);
            return way;
        } else {
            auto lru_way = lru_replacement_policy::miss();
            auto nested_way = nested_[lru_way].miss();
            return lru_way * nested_ways() + nested_way;
        }
    }
};

static_assert(replacement_policy<lru_wrapped_replacement_policy<plru_replacement_policy>>);

}

#endif /* CACHEHOUND_POLICIES_REPLACEMENT_LRU_WRAPPED_REPLACEMENT_POLICY_HPP */
