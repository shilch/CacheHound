#ifndef CACHEHOUND_ALGO_IMPL_REDUCE_EVICTION_SET_HPP
#define CACHEHOUND_ALGO_IMPL_REDUCE_EVICTION_SET_HPP

#include "../reduce_eviction_set.hpp"
#include "cachehound/algo/is_eviction_set.hpp"

#include <ranges>

template<
    cachehound::is_eviction_set Func,
    cachehound::instrumented_memory Memory,
    std::bidirectional_iterator BidirIt
>
[[nodiscard]] BidirIt cachehound::reduce_eviction_set(
    Func&& is_eviction_set,
    Memory& memory,
    std::uintptr_t target,
    BidirIt first,
    BidirIt last,
    std::size_t max_size
) {
    // For annotation purposes:
    // assert(is_eviction_set(memory, target, std::ranges::subrange(first, last)));

    // Desired number of groups
    // This must be at least one more than the max size of the eviction set
    // such that there exists at least one group which, when removed,
    // still makes it an eviction set.
    const std::size_t groups = max_size + 1;

    std::size_t current_size = std::distance(first, last);

    // Iteration by iteration: Identify the group that can be removed
    // to produce a smaller eviction set.
    while(current_size > max_size) {
        // Divide the groups into two categories:
        // (1) Groups with one extra member (these are the remaining members caused by division)
        // (2) Groups without an extra member
        std::size_t min_group_size = current_size / groups;
        std::size_t groups_with_extra_member = current_size % min_group_size;
        std::size_t groups_without_extra_member = groups - groups_with_extra_member;
        std::array<std::size_t, 2> group_counters { groups_with_extra_member, groups_without_extra_member };

        // Start with a mask for the bigger groups
        auto mask_begin = first;
        auto mask_end = std::next(first, min_group_size + 1);
        std::size_t group_size = min_group_size + 1;

        // For both types of groups
        for(auto& counter : group_counters) {
            // Iterate through all the groups, updating the mask along the way
            while(counter-- > 0) {
                // ... and check whether exclusion still produces a valid eviction set
                std::array sub_evset{
                    std::ranges::subrange(first, mask_begin),
                    std::ranges::subrange(mask_end, last)
                };
                bool is_evset = is_eviction_set(memory, target, std::ranges::join_view(sub_evset));
                if(is_evset) {
                    // If so, break out of the loop and jump
                    // to code which strips out the group from the set.
                    goto found;
                }

                std::advance(mask_begin, group_size);
                std::advance(mask_end, group_size);
            }

            // Adjust mask for smaller groups in next iteration
            --mask_end;
            --group_size;
        }

        return first; // error, reduction failed

    found:
        if (mask_end == last) {
            last = mask_begin;
        } else {
            // Overwrite group in mask with last group
            auto last_group_begin = std::next(last, -min_group_size);
            std::swap_ranges(last_group_begin, last, mask_begin);
            last = last_group_begin;
        }
        current_size -= min_group_size;
    }

    return last;
}

bool cachehound::reduce_eviction_set(
    is_eviction_set auto&& is_eviction_set,
    cachehound::instrumented_memory auto& memory,
    std::uintptr_t target,
    std::vector<std::uintptr_t>& eviction_set,
    std::size_t max_size
) {
    auto last = reduce_eviction_set(std::forward<decltype(is_eviction_set)>(is_eviction_set), memory, target, eviction_set.begin(), eviction_set.end(), max_size);
    if (last == eviction_set.begin()) return false;
    eviction_set.resize(std::distance(eviction_set.begin(), last));
    return true;
}

template<
    cachehound::is_eviction_set Func,
    cachehound::instrumented_memory Memory,
    std::bidirectional_iterator BidirIt
>
[[nodiscard]] BidirIt cachehound::reduce_eviction_set_slow(
    Func&& is_eviction_set,
    Memory& memory,
    std::uintptr_t target,
    BidirIt first,
    BidirIt last,
    std::size_t max_size
) {
    // For annotation purposes:
    // assert(is_eviction_set(memory, target, std::ranges::subrange(first, last)));

    // O(n^2) solution:
    auto crucial_begin = first;
    auto candidates_begin = first;
    auto candidates_end = last;

    while (std::distance(crucial_begin, candidates_end) > max_size) {
        if (std::distance(candidates_begin, candidates_end) == 0) {
            // Minimization failed because we have no more candidates left
            return first;
        }

        bool redundant_candidate = is_eviction_set(memory, target, std::ranges::subrange(crucial_begin, candidates_end - 1));

        if (redundant_candidate) {
            --candidates_end;
        } else {
            std::swap(*candidates_begin, *(candidates_end - 1));
            ++candidates_begin;
        }
    }

    return candidates_end;
}

bool cachehound::reduce_eviction_set_slow(
    is_eviction_set auto&& is_eviction_set,
    instrumented_memory auto& memory,
    std::uintptr_t target,
    std::vector<std::uintptr_t>& eviction_set,
    std::size_t max_size
) {
    auto last = reduce_eviction_set_slow(std::forward<decltype(is_eviction_set)>(is_eviction_set), memory, target, eviction_set.begin(), eviction_set.end(), max_size);
    if (last == eviction_set.begin()) return false;
    eviction_set.resize(std::distance(eviction_set.begin(), last));
    return true;
}

#endif /* CACHEHOUND_ALGO_IMPL_REDUCE_EVICTION_SET_HPP */
