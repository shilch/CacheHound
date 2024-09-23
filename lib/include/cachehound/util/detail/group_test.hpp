#ifndef CACHEHOUND_UTIL_DETAIL_GROUP_TEST_HPP
#define CACHEHOUND_UTIL_DETAIL_GROUP_TEST_HPP

#include <ranges>

namespace cachehound::detail {

[[nodiscard]] auto group_test(
    std::ranges::bidirectional_range auto&& range,
    auto&& test_func
) -> std::ranges::iterator_t<decltype(range)> {
    bool exists = std::forward<decltype(test_func)>(test_func)(range);
    if(!exists) {
        return std::forward<decltype(range)>(range).end();
    }

    auto err   = std::forward<decltype(range)>(range).end();
    auto first = std::forward<decltype(range)>(range).begin()
       , last  = std::forward<decltype(range)>(range).end();

    while(true) {
        auto dist = std::distance(first, last);
        if(dist <= 1) {
            assert(dist != 0);
            return first;
        }

        auto mid = std::next(first, dist / 2);

        if(std::forward<decltype(test_func)>(test_func)(std::ranges::subrange(first, mid))) {
            last = mid;
        } else {
            first = mid;
        }
    }
}

}

#endif /* CACHEHOUND_UTIL_DETAIL_GROUP_TEST_HPP */
