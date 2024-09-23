#ifndef CACHEHOUND_CLI_PLACEMENT_STACK_HPP
#define CACHEHOUND_CLI_PLACEMENT_STACK_HPP

#include <Eigen/Eigen>
#include <Eigen/src/Core/util/Constants.h>
#include <ranges>
#include <concepts>

namespace cachehound::cli {

template<typename ScalarType, std::ranges::forward_range Range>
    requires (std::convertible_to<std::ranges::range_value_t<Range>, Eigen::Vector<ScalarType, Eigen::Dynamic>>)
Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> hstack(Range&& vector_range) {
    Eigen::Vector<ScalarType, Eigen::Dynamic> first_vec = (*std::forward<Range>(vector_range).begin());

    std::size_t rows = first_vec.size();
    std::size_t cols = std::ranges::distance(vector_range);

    Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> mat(rows, cols);
    mat.col(0) = first_vec;

    std::size_t i = 1;
    for(auto&& val : std::forward<Range>(vector_range) | std::views::drop(1)) {
        Eigen::Vector<ScalarType, Eigen::Dynamic> vec = std::forward<decltype(val)>(val);
        assert(vec.size() == rows);
        mat.col(i) = std::move(vec);
        i++;
    }
    return mat;
}

}

#endif /* CACHEHOUND_CLI_PLACEMENT_STACK_HPP */
