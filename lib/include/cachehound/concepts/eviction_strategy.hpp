#ifndef CACHEHOUND_CONCEPTS_EVICTION_STRATEGY_HPP
#define CACHEHOUND_CONCEPTS_EVICTION_STRATEGY_HPP

#include "./memory.hpp"

#include <concepts>
#include <ranges>

namespace cachehound {

template<typename S>
concept eviction_strategy = requires(S strategy, std::uintptr_t address, std::ranges::subrange<std::size_t*> set_range) {
    { strategy.memory() } -> memory;
    { strategy.maps_to(address, set_range) } -> std::same_as<bool>;
};

}

#endif /* CACHEHOUND_CONCEPTS_EVICTION_STRATEGY_HPP */
