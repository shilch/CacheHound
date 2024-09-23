#ifndef CACHEHOUND_POLICIES_REPLACEMENT_MRU_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_REPLACEMENT_MRU_REPLACEMENT_POLICY_HPP

#include "./srrip_replacement_policy.hpp"
#include "../../concepts/replacement_policy.hpp"

namespace cachehound {

using mru_replacement_policy = srrip_replacement_policy<1, hit_priority>;
static_assert(replacement_policy<mru_replacement_policy>);


}

#endif /* CACHEHOUND_POLICIES_REPLACEMENT_MRU_REPLACEMENT_POLICY_HPP */
