#ifndef CACHEHOUND_CLI_PLACEMENT_AFFINELY_INDEPENDENT_HPP
#define CACHEHOUND_CLI_PLACEMENT_AFFINELY_INDEPENDENT_HPP

#include "./linearly_independent.hpp"
#include <Eigen/src/Core/util/Constants.h>
#include <ranges>

namespace cachehound::cli {

template<typename ScalarType, std::ranges::forward_range Range>
    requires (std::convertible_to<std::ranges::range_value_t<Range>, Eigen::Vector<ScalarType, Eigen::Dynamic>>)
bool affinely_independent(Range&& vectors) {
    std::size_t count = std::ranges::distance(vectors);
    assert(count > 1);

    Eigen::Vector<ScalarType, Eigen::Dynamic> first_vec = *std::forward<Range>(vectors).begin();
    return linearly_independent<ScalarType>(
        std::forward<Range>(vectors) |
        std::views::drop(1) |
        std::views::transform([&](auto&& in){
            Eigen::Vector<ScalarType, Eigen::Dynamic> vec = std::forward<decltype(in)>(in);
            vec += first_vec;
            return vec;
        })
    );
}

}

#endif /* CACHEHOUND_CLI_PLACEMENT_AFFINELY_INDEPENDENT_HPP */
