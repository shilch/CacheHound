#ifndef CACHEHOUND_ALGO_IS_EVICTION_SET_HPP
#define CACHEHOUND_ALGO_IS_EVICTION_SET_HPP

#include "../concepts/address_range.hpp"
#include "../concepts/instrumented_memory.hpp"
#include "../backends/kernel_memory.hpp"

namespace cachehound {

template<typename F>
concept is_eviction_set = requires(F func, kernel_memory memory, std::uintptr_t target, std::ranges::subrange<std::uintptr_t*> addresses) {
    { func(memory, target, addresses) } -> std::same_as<bool>;
};

/**
 * @brief Determines whether the given ranges of addresses
 * constitute an eviction set for the specified target address.
 * This is just a minimum/bare variant and should be used in
 * conjunction with memory reset methods and/or cache polluting.
 *
 * The safe variant verifies that all accesses perform an first level
 * miss. It is recommended when using cache polluting to make sure
 * that the polluting was sufficient. A mechanism around the safe variant
 * is required to satisfy the is_eviction_set concept (which expects a
 * bool return type).
 * 
 * @param memory The memory to perform the accesses on
 * @param target The target address that should be evicted
 * @param addresses One or multiple ranges of addresses
 * @return true If the ranges of addresses are an eviction set for target
 * @return false Otherwise
 */
[[nodiscard]] bool unsafe_is_eviction_set(
    instrumented_memory auto& memory,
    std::uintptr_t target,
    address_range auto&& addresses
);

[[nodiscard]] std::optional<bool> safe_is_eviction_set(
    instrumented_memory auto& memory,
    std::uintptr_t target,
    address_range auto&& addresses
);

}

#include "./impl/is_eviction_set.hpp"

#endif /* CACHEHOUND_ALGO_IS_EVICTION_SET_HPP */
