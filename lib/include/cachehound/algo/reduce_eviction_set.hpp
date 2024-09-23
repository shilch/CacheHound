#ifndef CACHEHOUND_ALGO_REDUCE_EVICTION_SET_HPP
#define CACHEHOUND_ALGO_REDUCE_EVICTION_SET_HPP

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <vector>

#include "./is_eviction_set.hpp"
#include "../concepts/instrumented_memory.hpp"

namespace cachehound {

/**
 * @brief Reduces an eviction set to the specified size
 *
 * The caller must guarantee that the given range of addresses
 * are indeed an eviction set for the specified target address,
 * otherwise the behaviour is undefined.
 *
 * The algorithm runs in linear time according to the number of
 * addresses in the eviction set.
 *
 * @note This function may change the order of addresses in the input range.
 *
 * @param memory The memory to perform the accesses on
 * @param target The target address for that eviction set
 * @param first Iterator to the first address of the eviction set
 * @param last Iterator to the end of the eviction set (i. e., last address + 1)
 * @param max_size The desired maximum size of the eviction set after reduction
 * @return BidirIt Iterator to the end of the reduced eviction set, or first if the reduction failed
 */
template<is_eviction_set Func, instrumented_memory Memory, std::bidirectional_iterator BidirIt>
[[nodiscard]] BidirIt reduce_eviction_set(
    Func&& is_eviction_set,
    Memory& memory,
    std::uintptr_t target,
    BidirIt first,
    BidirIt last,
    std::size_t max_size
);

bool reduce_eviction_set(
    is_eviction_set auto&& is_eviction_set,
    instrumented_memory auto& memory,
    std::uintptr_t target,
    std::vector<std::uintptr_t>& eviction_set,
    std::size_t max_size
);

/**
 * @brief Reduces an eviction set to the specified size (quadratic solution)
 *
 * The caller must guarantee that the given range of addresses
 * are indeed an eviction set for the specified target address,
 * otherwise the behaviour is undefined.
 *
 * The algorithm runs in quadratic time according to the number of
 * addresses in the eviction set.
 *
 * @note This function may change the order of addresses in the input range.
 *
 * @param memory The memory to perform the accesses on
 * @param target The target address for that eviction set
 * @param first Iterator to the first address of the eviction set
 * @param last Iterator to the end of the eviction set (i. e., last address + 1)
 * @param max_size The desired maximum size of the eviction set after reduction
 * @return BidirIt Iterator to the end of the reduced eviction set, or first if the reduction failed
 */
template<is_eviction_set Func, instrumented_memory Memory, std::bidirectional_iterator BidirIt>
[[nodiscard]] BidirIt reduce_eviction_set_slow(
    Func&& is_eviction_set,
    Memory& memory,
    std::uintptr_t target,
    BidirIt first,
    BidirIt last,
    std::size_t max_size
);

bool reduce_eviction_set_slow(
    is_eviction_set auto&& is_eviction_set,
    instrumented_memory auto& memory,
    std::uintptr_t target,
    std::vector<std::uintptr_t>& eviction_set,
    std::size_t max_size
);

}

#include "./impl/reduce_eviction_set.hpp"

#endif /* CACHEHOUND_ALGO_REDUCE_EVICTION_SET_HPP */
