#ifndef CACHEHOUND_POLICIES_REPLACEMENT_QLRU_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_REPLACEMENT_QLRU_REPLACEMENT_POLICY_HPP

#include "./srrip_replacement_policy.hpp"

namespace cachehound {

template<hit_promotion_policy HitPromotionPolicy>
using qlru_replacement_policy = srrip_replacement_policy<2, HitPromotionPolicy>;
static_assert(replacement_policy<qlru_replacement_policy<hit_priority>>);

}

#endif /* CACHEHOUND_POLICIES_REPLACEMENT_QLRU_REPLACEMENT_POLICY_HPP */
