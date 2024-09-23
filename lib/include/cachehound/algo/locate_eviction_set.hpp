#ifndef CACHEHOUND_ALGO_LOCATE_EVICTION_SET_HPP
#define CACHEHOUND_ALGO_LOCATE_EVICTION_SET_HPP

#include "../concepts/instrumented_memory.hpp"
#include "./is_eviction_set.hpp"
#include "../util/detail/group_test.hpp"

#include <ranges>

namespace cachehound {

[[nodiscard]] auto locate_eviction_set(
    is_eviction_set auto&& is_eviction_set,
    instrumented_memory auto& memory,
    std::uintptr_t target,
    std::ranges::bidirectional_range auto&& range
) -> std::ranges::iterator_t<decltype(range)> {
    return detail::group_test(range, [
        &memory,
        target,
        is_eviction_set = std::forward<decltype(is_eviction_set)>(is_eviction_set)
    ](auto&& evset_range) mutable {
        return std::forward<decltype(is_eviction_set)>(is_eviction_set)(
            memory,
            target,
            std::ranges::join_view(std::forward<decltype(evset_range)>(evset_range))
        );
    });
}

}

#endif /* CACHEHOUND_ALGO_LOCATE_EVICTION_SET_HPP */
