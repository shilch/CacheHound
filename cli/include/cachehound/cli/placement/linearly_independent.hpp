#ifndef CACHEHOUND_CLI_PLACEMENT_LINEARLY_INDEPENDENT_HPP
#define CACHEHOUND_CLI_PLACEMENT_LINEARLY_INDEPENDENT_HPP

#include <ranges>
#include <Eigen/Eigen>
#include "./bit.hpp"
#include "./determinant.hpp"
#include "./stack.hpp"

namespace cachehound::cli {

template<typename ScalarType, std::ranges::forward_range Range>
    requires (std::convertible_to<std::ranges::range_value_t<Range>, Eigen::Vector<ScalarType, Eigen::Dynamic>>)
bool linearly_independent(Range&& vectors) {
    auto matrix = hstack<ScalarType>(std::forward<Range>(vectors));
    assert(matrix.rows() == matrix.cols());
    assert(matrix.rows() > 1);
    return determinant(matrix) != ScalarType{};
}

}

#endif /* CACHEHOUND_CLI_PLACEMENT_LINEARLY_INDEPENDENT_HPP */
