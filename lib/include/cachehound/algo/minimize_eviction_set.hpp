#ifndef CACHEHOUND_ALGO_MINIMIZE_EVICTION_SET_HPP
#define CACHEHOUND_ALGO_MINIMIZE_EVICTION_SET_HPP

#include "./reduce_eviction_set.hpp"
#include "./is_eviction_set.hpp"

namespace cachehound {

/**
 * @brief Minimizes an eviction set to the smallest possible size
 *
 * The caller must guarantee that the given range of addresses
 * are indeed an eviction set for the specified target address,
 * otherwise the behaviour is undefined.
 *
 * The algorithm runs in O(n log k) where n is the number of addresses
 * in the eviction set and k is max_size.
 *
 * @note This function may change the order of addresses in the input range.
 *
 * @param memory The memory to perform the accesses on
 * @param target The target address for that eviction set
 * @param first Iterator to the first address of the eviction set
 * @param last Iterator to the end of the eviction set (i. e., last address + 1)
 * @param max_size An educated guess on the upper bound of the eviction set size; in many cases 16 is a reasonable choice
 * @return BidirIt Iterator to the end of the minimized eviction set, or first if the minimization failed (in which case max_size might need to be increased)
 */
template <is_eviction_set Func, instrumented_memory Memory, std::bidirectional_iterator BidirIt>
BidirIt minimize_eviction_set(
    Func&& is_eviction_set,
    Memory& memory,
    std::uintptr_t target,
    BidirIt first,
    BidirIt last,
    std::size_t max_size)
{
    // For annotation purposes:
    // assert(is_eviction_set(memory, target, std::ranges::subrange(first, last)));

    // Reduction to upper bound
    auto new_last = reduce_eviction_set(std::forward<Func>(is_eviction_set), memory, target, first, last, max_size);
    if (new_last == first)
        return first;
    last = new_last;

    // Binary search for lower bound eviction set size
    std::size_t l = 1;
    std::size_t r = max_size;
    BidirIt it;

    while (l < r) {
        auto m = l + (r - l) / 2;

        auto reduction_it = reduce_eviction_set(memory, target, first, last, m);
        bool reduction_successful = reduction_it != first;
        if (reduction_successful) {
            r = m;
            it = reduction_it;
        } else {
            l = m + 1;
        }
    }

    return it;
}

template <instrumented_memory Memory>
bool minimize_eviction_set(
    Memory& memory,
    std::uintptr_t target,
    std::vector<std::uintptr_t>& eviction_set,
    std::size_t max_size)
{
    auto last = minimize_eviction_set(memory, target, eviction_set.begin(), eviction_set.end(), max_size);
    if (last == eviction_set.begin())
        return false;
    eviction_set.resize(std::distance(eviction_set.begin(), last));
    return true;
}

/**
 * @brief Minimizes an eviction set to the smallest possible size (quadratic solution)
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
 * @param max_size An educated guess on the upper bound of the eviction set size; in many cases 16 is a reasonable choice
 * @return BidirIt Iterator to the end of the minimized eviction set, or first if the minimization failed (in which case max_size might need to be increased)
 */
template <is_eviction_set Func, instrumented_memory Memory, std::bidirectional_iterator BidirIt>
BidirIt minimize_eviction_set_slow(
    Func&& is_eviction_set,
    Memory& memory,
    std::uintptr_t target,
    BidirIt first,
    BidirIt last)
{
    // For annotation purposes:
    // assert(is_eviction_set(memory, target, std::ranges::subrange(first, last)));

    // O(n^2) solution:
    auto crucial_begin = first;
    auto candidates_begin = first;
    auto candidates_end = last;

    while (std::distance(candidates_begin, candidates_end) != 0) {
        bool redundant_candidate = std::forward<Func>(is_eviction_set)(memory, target, std::ranges::subrange(crucial_begin, candidates_end - 1));

        if (redundant_candidate) {
            --candidates_end;
        } else {
            std::swap(*candidates_begin, *(candidates_end - 1));
            ++candidates_begin;
        }
    }

    return candidates_end;
}

bool minimize_eviction_set_slow(
    is_eviction_set auto&& is_eviction_set,
    instrumented_memory auto& memory,
    std::uintptr_t target,
    std::vector<std::uintptr_t>& eviction_set)
{
    auto last = minimize_eviction_set_slow(
        std::forward<decltype(is_eviction_set)>(is_eviction_set),
        memory,
        target,
        eviction_set.begin(),
        eviction_set.end()
    );
    if (last == eviction_set.begin())
        return false;
    eviction_set.resize(std::distance(eviction_set.begin(), last));
    return true;
}

}

#endif /* CACHEHOUND_ALGO_MINIMIZE_EVICTION_SET_HPP */
