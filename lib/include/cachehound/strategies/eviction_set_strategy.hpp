#ifndef CACHEHOUND_STRATEGIES_EVICTION_SET_STRATEGY_HPP
#define CACHEHOUND_STRATEGIES_EVICTION_SET_STRATEGY_HPP

#include "../concepts/instrumented_memory.hpp"
#include "../concepts/eviction_strategy.hpp"
#include "../algo/is_eviction_set.hpp"

#include <ranges>
#include <unordered_set>
#include <vector>

namespace cachehound {

template<
    instrumented_memory Memory,
    is_eviction_set IsEvictionSet
>
class eviction_set_strategy {
    Memory& memory_;
    std::vector<std::vector<std::uintptr_t>> evsets_;
    IsEvictionSet is_eviction_set_;

public:
    eviction_set_strategy(
        Memory& memory,
        std::vector<std::vector<std::uintptr_t>> evsets,
        IsEvictionSet is_eviction_set = {}
    )   : memory_(memory)
        , evsets_(std::move(evsets))
        , is_eviction_set_(std::move(is_eviction_set)) {}

    Memory& memory() {
        return memory_;
    }

    bool maps_to(std::uintptr_t target, std::ranges::input_range auto&& set_range)
        requires std::convertible_to<std::ranges::range_value_t<decltype(set_range)>, std::size_t>
    {
        std::vector<std::uintptr_t> evset_addresses;
        for(std::size_t set : std::forward<decltype(set_range)>(set_range)) {
            assert(set < evsets_.size());

            for(std::uintptr_t set_address : evsets_[set]) {
                evset_addresses.emplace_back(set_address);
            }
        }

        return is_eviction_set_(memory_, target, evset_addresses);
    }
};

}

#endif /* CACHEHOUND_STRATEGIES_EVICTION_SET_STRATEGY_HPP */
