#ifndef CACHEHOUND_UTIL_LOCATE_SET_HPP
#define CACHEHOUND_UTIL_LOCATE_SET_HPP

#include "../concepts/eviction_strategy.hpp"
#include "./detail/group_test.hpp"

#include <ranges>
#include <optional>

namespace cachehound {

std::optional<std::size_t> locate_set(eviction_strategy auto& strategy, std::uintptr_t target) {
    std::size_t sets = 1 << strategy.memory().index_bits(0);
    std::ranges::iota_view set_range{std::size_t{0}, sets};
    auto set = *detail::group_test(set_range, [&strategy, target](auto&& set_range){
        return strategy.maps_to(target, std::forward<decltype(set_range)>(set_range));
    });
    if (set == sets) return std::nullopt;
    return set;
}

}

#endif /* CACHEHOUND_UTIL_LOCATE_SET_HPP */
