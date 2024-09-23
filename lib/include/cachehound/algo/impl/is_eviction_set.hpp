#ifndef CACHEHOUND_ALGO_IMPL_IS_EVICTION_SET_HPP
#define CACHEHOUND_ALGO_IMPL_IS_EVICTION_SET_HPP

#include "../is_eviction_set.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>

bool cachehound::unsafe_is_eviction_set(
    cachehound::instrumented_memory auto& memory,
    std::uintptr_t target,
    cachehound::address_range auto&& addresses
) {
    memory.access(target);
    for(std::uintptr_t address : std::forward<decltype(addresses)>(addresses)) {
        memory.access(address);
    }
    return memory.instrumented_access(target) > 0;
}

std::optional<bool> cachehound::safe_is_eviction_set(
    cachehound::instrumented_memory auto& memory,
    std::uintptr_t target,
    cachehound::address_range auto&& addresses
) {
    if(memory.instrumented_access(target) == 0) {
        return std::nullopt;
    }
    for(std::uintptr_t address : std::forward<decltype(addresses)>(addresses)) {
        if(memory.instrumented_access(address) == 0) {
            return std::nullopt;
        }
    }
    return memory.instrumented_access(target) > 0;
}

#endif /* CACHEHOUND_ALGO_IMPL_IS_EVICTION_SET_HPP */
