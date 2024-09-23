#ifndef CACHEHOUND_POLICIES_REPLACEMENT_LRU_PLRU_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_REPLACEMENT_LRU_PLRU_REPLACEMENT_POLICY_HPP

#include "./lru_wrapped_replacement_policy.hpp"

namespace cachehound {

class lru_plru_replacement_policy : public lru_wrapped_replacement_policy<plru_replacement_policy> {
public:
    lru_plru_replacement_policy(std::size_t lru_ways, std::size_t plru_ways)
        : lru_wrapped_replacement_policy<plru_replacement_policy>(lru_ways, plru_replacement_policy{plru_ways}) {}
};

}

#endif /* CACHEHOUND_POLICIES_REPLACEMENT_LRU_PLRU_REPLACEMENT_POLICY_HPP */
