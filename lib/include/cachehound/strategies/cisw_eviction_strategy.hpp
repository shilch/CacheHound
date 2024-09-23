#ifndef CACHEHOUND_STRATEGIES_CISW_EVICTION_STRATEGY_HPP
#define CACHEHOUND_STRATEGIES_CISW_EVICTION_STRATEGY_HPP

#include "../concepts/armv8_memory.hpp"
#include "../concepts/instrumented_memory.hpp"
#include "../concepts/eviction_strategy.hpp"
#include "../algo/is_eviction_set.hpp"

#include <ranges>

namespace cachehound {

template<armv8_memory Memory>
    requires instrumented_memory<Memory>
class cisw_eviction_strategy {
    Memory& memory_;

public:
    cisw_eviction_strategy(Memory& memory) : memory_(memory) {}

    Memory& memory() {
        return memory_;
    }

    bool maps_to(std::uintptr_t target, std::ranges::input_range auto&& set_range)
        requires std::convertible_to<std::ranges::range_value_t<decltype(set_range)>, std::size_t>
    {
        memory_.access(target);
        for(std::size_t set : std::forward<decltype(set_range)>(set_range)) {
            assert(set < (1 << memory_.index_bits(0)));
            for(std::size_t way = 0; way < memory_.ways(0); way++) {
                memory_.cisw(0, set, way);
            }
        }
        return memory_.instrumented_access(target) > 0;
    }
};

}

#endif /* CACHEHOUND_STRATEGIES_CISW_EVICTION_STRATEGY_HPP */
