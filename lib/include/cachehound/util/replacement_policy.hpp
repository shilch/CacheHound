#ifndef CACHEHOUND_UTIL_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_UTIL_REPLACEMENT_POLICY_HPP

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>
#include <unordered_set>

#include "./any_replacement_policy.hpp"
#include "../concepts/replacement_policy.hpp"
#include "../concepts/sequence.hpp"

#include "../policies/replacement/fifo_replacement_policy.hpp"
#include "../policies/replacement/linear_init_replacement_policy.hpp"
#include "../policies/replacement/lru_plru_replacement_policy.hpp"
#include "../policies/replacement/lru_replacement_policy.hpp"
#include "../policies/replacement/mru_replacement_policy.hpp"
#include "../policies/replacement/plru_replacement_policy.hpp"
#include "../policies/replacement/qlru_replacement_policy.hpp"
#include "../policies/replacement/srrip_replacement_policy.hpp"

namespace cachehound {

inline std::vector<std::string> get_replacement_policies(std::size_t associativity) {
    assert(associativity > 0);

    std::vector<std::string> candidates;
    candidates.emplace_back("FIFO");
    candidates.emplace_back("LRU");
    if((associativity & (associativity - 1)) == 0) {
        candidates.emplace_back("PLRU");
        candidates.emplace_back("PLRUl");
    }
    for(std::size_t plru_associativity = 4; plru_associativity < associativity; plru_associativity *= 2) {
        if(associativity % plru_associativity != 0) continue;

        std::size_t lru_associativity = associativity / plru_associativity;

        std::string name = "LRU";
        // name += std::to_string(lru_associativity);
        name += "_PLRU";
        name += std::to_string(plru_associativity);

        candidates.emplace_back(std::move(name));
    }
    candidates.emplace_back("SRRIP");
    return candidates;
}

inline std::optional<any_replacement_policy> make_replacement_policy(std::string_view name, std::size_t associativity) {
    if(name == "FIFO") {
        return fifo_replacement_policy{associativity};
    } else if(name == "LRU") {
        return lru_replacement_policy{associativity};
    } else if(name == "NRU") {
        return mru_replacement_policy{associativity};
    } else if((name == "PLRU" || name == "PLRUl") && (associativity != 0 && (((associativity - 1) & associativity) == 0))) {
        // if(name == "PLRU") {
            return plru_replacement_policy{associativity};
        // } else {
        //     return linear_init_replacement_policy{plru_replacement_policy{associativity}};
        // }
    } else if(name.starts_with("LRU_PLRU")) {
        std::size_t plru_ways = std::stoi(std::string{name.substr(8)});
        return lru_plru_replacement_policy{associativity / plru_ways, plru_ways};
    } else if(name == "SRRIP") {
        return srrip_replacement_policy<2, hit_priority>{associativity};
    }
    return std::nullopt;
}

template<replacement_policy Policy>
bool check_cyclic_eviction(Policy& policy) {
    auto associativity = policy.ways();
    std::unordered_set<std::size_t> evicted_ways;

    for(std::size_t i = 0; i < associativity; i++) {
        evicted_ways.emplace(policy.miss());
    }

    return evicted_ways.size() == associativity;
}

}

#endif /* CACHEHOUND_UTIL_REPLACEMENT_POLICY_HPP */
