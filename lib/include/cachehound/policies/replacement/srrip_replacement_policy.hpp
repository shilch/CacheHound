#ifndef CACHEHOUND_POLICIES_REPLACEMENT_SRRIP_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_REPLACEMENT_SRRIP_REPLACEMENT_POLICY_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <numeric>
#include <vector>

#include "../../concepts/replacement_policy.hpp"

namespace cachehound {

using hit_promotion_policy = std::size_t(std::size_t);

constexpr std::size_t hit_priority(std::size_t in) {
    return 0;
}

constexpr std::size_t frequency_priority(std::size_t in) {
    if (in == 0) return 0;
    return in - 1;
}

template<
    std::size_t Bits,
    hit_promotion_policy HitPromotionPolicy
>
class srrip_replacement_policy {
    std::vector<std::size_t> entries_;

    static constexpr std::size_t distant_future        = (static_cast<std::size_t>(1) << Bits) - 1;
    static constexpr std::size_t near_immediate_future = 0x0;
    static constexpr std::size_t long_interval         = distant_future - 1;

public:
    srrip_replacement_policy(std::size_t ways) : entries_(ways, distant_future) {
        assert(ways > 0);
    }

    std::size_t ways() const noexcept {
        return entries_.size();
    }

    void hit(std::size_t way) noexcept {
        entries_[way] = HitPromotionPolicy(entries_[way]);
    }

    std::size_t miss() noexcept {
        while(true) {
            // Find first entry from the left with distant future
            auto it = std::find(entries_.begin(), entries_.end(), distant_future);
            if (it != entries_.cend()) {
                // Use long interval (i. e., max - 1) as initial prediction for new element
                *it = long_interval;
                return std::distance(entries_.begin(), it);
            }

            // Increment all entries by one
            std::transform(entries_.cbegin(), entries_.cend(), entries_.begin(), [](auto entry) {
                return entry + 1;
            });
        }
    }
};
static_assert(replacement_policy<srrip_replacement_policy<1, hit_priority>>);
static_assert(replacement_policy<srrip_replacement_policy<1, frequency_priority>>);

}

#endif /* CACHEHOUND_POLICIES_REPLACEMENT_SRRIP_REPLACEMENT_POLICY_HPP */
